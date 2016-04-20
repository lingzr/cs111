#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <getopt.h>
#include <mcrypt.h>


struct termios save_attr;

/*
        two buffers to store the data received / send from the client side socket
    */

int sockfd;    
int max_size = 10000;
char *buffer_received;
char *buffer_sent;
int buffer_received_ptr = 0;
int buffer_sent_ptr = 0;
int flag_encrypt;
char* log_file_name = NULL;
char* port_num = NULL;

struct termios save_attr;


void reset_input_mode (void);

//set input mode
void set_input_mode (void);
//handlers:
//SIGPIPE handler
void pipe_handler(int signum);
void int_handler (int signum);
void exit_handler (void);

// mcrypt_generic (td, &block_buffer, 1);
// mdecrypt_generic (td, &block_buffer, 1);

MCRYPT TD;


MCRYPT encrypt_init ()
{
  MCRYPT td;
  int i;
  char *key;
  //char password[20];
  //char block_buffer;
  char *IV;
  int keysize=7; /* 128 bits */
  key=calloc(1, keysize);
  //strcpy(password, "A_large_key");
/* Generate the key using the password */
/*  mhash_keygen( KEYGEN_MCRYPT, MHASH_MD5, key, keysize, NULL, 0, password,
strlen(password));
 */
  //memmove( key, password, strlen(password));

    
//read the key from my.key
   FILE *fp;
  

   fp = fopen("my.key", "r");
   

   fgets(key, 16, (FILE*)fp);
   
   
   fclose(fp);


   td = mcrypt_module_open("des", NULL, "cfb", NULL);
  
  IV = malloc(mcrypt_enc_get_iv_size(td));
/* Put random data in IV. Note these are not real random data,
 * consider using /dev/random or /dev/urandom.
 */
  /*  srand(time(0)); */
  for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
    IV[i]=i;
  }

  i=mcrypt_generic_init( td, key, keysize, IV);
  if (i<0) {
     mcrypt_perror(i);
     
  }
  return td;
}



void error(char *msg)
{
    perror(msg);
    exit(0);
}



//this thread reads from shell and output to the stdout
void* thread_func (void *fd){
    char buf[128];
    int size = 0;

    //need to decrypt!!!!!
    //(size = read(*(int *)fd, buf, 1))>0

    while ((size = read(*(int *)fd, buf, 1))>0)
    {
    //upon receiving EOF from the shell
    
    
//fprintf(fp, "catch2\n");

        //store to the buffer_received
        if (buffer_received_ptr >= max_size-1)
        {
             buffer_sent = (char *) realloc(buffer_sent, max_size*2);
             buffer_received = (char *) realloc(buffer_received, max_size*2);
             max_size = max_size*2;
        }

        

        buffer_received[buffer_received_ptr++] = buf[0];
       
        if (flag_encrypt)
        {
            mdecrypt_generic (TD, buf, 1);
        }


        
        write (1, buf, size);
    
                    
    }


    close(sockfd);
    reset_input_mode();
    exit(1);

}



int main(int argc, char *argv[])
{
    set_input_mode();
    
    atexit(exit_handler);
    
    buffer_received =(char*)malloc(max_size* sizeof(char) );
    buffer_sent = (char*)malloc(max_size* sizeof(char) );


    TD = encrypt_init();

    /*
        parsing the argument
    */

    
    
    
    
    int arg;

    while (1)
    {
      static struct option long_options[] =
        {
          //set flag
          {"encrypt", no_argument,  &flag_encrypt, 1},
        
          
          //set value
          
          {"log",  required_argument, 0, 'l'},
          {"port",  required_argument, 0, 'p'},
          
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      arg = getopt_long (argc, argv, "lp",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (arg == -1)
        break;

      switch (arg)
        {
        
        case 0:
          break;

        case 'l':
          log_file_name = optarg;
          break;

        case 'p':
          port_num = optarg;
          break;


        default:
          return 0;


        }
    }




    int  portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    // if (argc < 3) {
    //    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    //    exit(0);
    // }

    if (port_num==0)
    {
        printf("please specify port number!\n");
        exit(0);
    }

    portno = atoi(port_num);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");


    //the new thread is responsible for 
    //reading from the socket and write to the stdout
    pthread_t thread1;

    pthread_create(&thread1, NULL, &thread_func, &sockfd);

    //printf("Please enter the message: ");
    bzero(buffer,1);


   

    // fgets(buffer,1,stdin);
    // n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    //bzero(buffer,1);
    // n = read(sockfd,buffer,1);
    // if (n < 0) 
    //      error("ERROR reading from socket");
    //printf("%s\n",buffer);





     //read fron stdin and write to the socket
    while (read(0, buffer,1)>0)
    {
        //store to the buffer_sent
        if (buffer_sent_ptr >= max_size-1)
        {
             buffer_sent = (char *) realloc(buffer_sent, max_size*2);
             buffer_received = (char *) realloc(buffer_received, max_size*2);
             max_size = max_size*2;
        }

        if (buffer[0]==4)
        {
            close(sockfd);
            reset_input_mode();
            exit(0);

        }

        //print to the screen
        write(1, buffer, 1);

        if (flag_encrypt)
        {
            mcrypt_generic (TD, buffer, 1);
        }
        
        if (strlen(buffer)== 0)
        {
            buffer[0]=' ';
        }
        buffer_sent[buffer_sent_ptr] = buffer[0];
        buffer_sent_ptr++;
                    
        //need to encrypt!!
        // fprintf(fp, "catch1\n");

        

        write (sockfd, buffer, 1);

 
    }

   // printf("!!!!!!!!!!!"); 



     
    


    return 0;
}





void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &save_attr);
}



//set input mode

void set_input_mode (void)
{
    //saved attribute
        struct termios tattr;
        
        /* save the terminal attribute */
        tcgetattr (STDIN_FILENO, &tattr);
        //atexit(reset_input_mode);


        //set the terminal mode
        tcgetattr (0, &save_attr);


        
        tattr.c_lflag &= ~(ICANON|ECHO);

        tattr.c_cc[VTIME]=0;
        tattr.c_cc[VMIN] = 1;

        tcsetattr(0, TCSAFLUSH, &tattr);

}

//handlers:

//SIGPIPE handler

void pipe_handler(int signum)
{
    //restore the terminal mode
   // reset_input_mode();
    //printf("mother fucker" );
    exit (1);

}

// void int_handler (int signum)
// {
//     kill(pid, SIGINT);
// }

void exit_handler (void)
{
    if (log_file_name!=NULL)
    {
       // printf("%s\n",buffer_received );
        //printf("wtf!!!!!!!\n");
        //printf("!!!!!!!!!!!");
        int received_byte = strlen(buffer_received);
        int sent_byte = strlen(buffer_sent);
        //write to the file
        FILE *f = fopen(log_file_name, "w");
        //int f = fopen(log_file_name, "w");
        if (f == NULL)
        {
            printf("Error opening file!\n");
            exit(1);
        }

        /* print some text */
        // fprintf(f, "SENT %d bytes: ",sent_byte );
        // write (f, buffer_sent, sent_byte);
        // fprintf(f, "\n" );
        // fprintf(f, "RECEIVED %d bytes: ",received_byte );
        // write (f, buffer_received, received_byte);
        // fprintf(f, "\n" );

        fprintf(f, "SENT %d bytes: %s\nRECEIVED %d bytes: %s\n",buffer_sent_ptr,buffer_sent,buffer_received_ptr,buffer_received);

        

        fclose(f);
    }
}
