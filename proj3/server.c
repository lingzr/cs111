#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <mcrypt.h>

int flag_encrypt;
int fp;
struct termios save_attr;
int pid;
int sockfd;

void reset_input_mode (void);
//this thread reads from shell and output to the stdout
void* thread_func (void *fd);
//set input mode
void set_input_mode (void);
//handlers:
//SIGPIPE handler
void pipe_handler(int signum);
void int_handler (int signum);
void exit_handler (void);

void error(char *msg)
{
    perror(msg);
    exit(1);
}

MCRYPT TD;


MCRYPT encrypt_init ()
{
  MCRYPT td;
  int i;
  char *key;
  //char password[20];
  //char block_buffer;
  char *IV;
  int keysize=16; /* 128 bits */
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


   td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  
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




int main (int argc, char* argv[])
{
    //parsing the argument
    /*
        parsing the argument
    */

    
    
    
    char* port_num = NULL;
    
    int arg;

    while (1)
    {
      static struct option long_options[] =
        {
          //set flag
          {"encrypt", no_argument,  &flag_encrypt, 1},
        
          
          //set value
          
          
          {"port",  required_argument, 0, 'p'},
          
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      arg = getopt_long (argc, argv, "p",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (arg == -1)
        break;

      switch (arg)
        {
        
        case 0:
          break;


        case 'p':
          port_num = optarg;
          break;


        default:
          return 0;


        }
    }

    

    TD = encrypt_init();
        
    // set_input_mode();

     int  newsockfd, portno, clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(port_num);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (unsigned int *) &clilen);



    
        atexit(exit_handler);

        int pipe_from_shell[2]; //filedexcriptor readend[0] filedescriptorforwriteend[1]
        int pipe_to_shell [2];

        pipe (pipe_from_shell);
        pipe (pipe_to_shell);

        pid = fork();

        if (pid==0){ //shell process

            close (pipe_from_shell[0]);//close the read end pipe from shell
            dup2(pipe_from_shell[1],1);

            close(pipe_to_shell[1]);
            dup2(pipe_to_shell[0], 0);

            execl("/bin/bash", "/bin/bash", NULL);

        }

        else{ //terminal proess
            //setup SIGPIPE handler using 'signal'
            signal (SIGPIPE, pipe_handler);
            //setup SIGINT C^C
            //signal (SIGINT, int_handler);
            //terminal does not use the eandofpipe_to_shell
            close (pipe_to_shell[0]);
            //terminal does not use writeend of pipe from shell pipe(pipefrom_shell)
            close (pipe_from_shell[1]);
            dup2( newsockfd , 1 );  /* duplicate socket on stdout */
            dup2( newsockfd , 2 );  /* duplicate socket on stderr too */
            dup2( newsockfd , 0 );  /* duplicate socket on stdin too */

            //create thread variable
            pthread_t thread1;

            pthread_create(&thread1, NULL, &thread_func, &pipe_from_shell[0]);

            //main thread, read from keyboard and write to stdout and forward to shell
            char buf[128];
            int size = 0;

            bzero(buffer,256);

            
            

                //need to decrypt
                while (read(0, buffer,1)>0)
                {
                    
                   // fprintf(fp, "catch1\n");
                    if (flag_encrypt)
                    {
                        mdecrypt_generic (TD, buffer, 1);
                    }

                   

                     
                     write (pipe_to_shell[1], buffer, 1);
 

                }
                close(sockfd);
                kill(pid, SIGHUP);
                exit(1);
                 

            }


}


    

    
   




void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &save_attr);
}




//this thread reads from shell and output to the stdout
void* thread_func (void *fd){
    char buf[128];
    int size = 0;

    while ((size = read(*(int *)fd, buf, 1))>0){
    //upon receiving EOF from the shell
    if (buf[0]==4){
        //restore the terminal mode and end;
        //reset_input_mode();
        //printf("mother\n");
        close(sockfd);
        //kill(pid, SIGHUP);
        exit(2);
    }
    else{
//fprintf(fp, "catch2\n");

        //need to encrypt
        if (flag_encrypt)
        {
            mcrypt_generic (TD, buf, 1);
        }

      
        
        write (1, buf, size);
    }
                    
    }

    return NULL;

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
    close(sockfd);
    kill(pid, SIGHUP);
    exit(2);

}



void exit_handler (void)
{
    //reset_input_mode();
}




