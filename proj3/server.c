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



int fp;
struct termios save_attr;
int pid;

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




int main (int argc, char* argv[])
{
    //parsing the argument
    


        
    // set_input_mode();

     int sockfd, newsockfd, portno, clilen;
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
     portno = atoi(argv[1]);
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
            signal (SIGINT, int_handler);
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

            
            
                //fp = fopen("file.txt", "w+")
                // if (buf[0]==4)
                // {
                //     //reach the edn of the file, call the EOF_handler
                //     //printf("mother fucker!!\n");
                //     close (pipe_to_shell[1]);
                //     close (pipe_from_shell[0]);
                //     kill(pid, SIGHUP);
                //     reset_input_mode();
                //     exit (0);
                // }
                // else if (buf[0] ==13 || buf[0] ==10)
                // {
                //     char cr =13;
                //     char lf =10;
                //     write(1, &cr, size);
                //     write(1, &lf, size);
                //     write (pipe_to_shell[1], &lf, size);

                // }
                // else
                // {
                //     write (1, buf, size);
                //     write (pipe_to_shell[1], buf, size);
                // }

                //need to decrypt
                while (read(0, buffer,1)>0)
                {
                    
                   // fprintf(fp, "catch1\n");
                     write (pipe_to_shell[1], buffer, 1);
 

                }
                 

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
        exit(1);
    }
    else{
//fprintf(fp, "catch2\n");

        //need to encrypt
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
    //restore the terminal mode
   // reset_input_mode();
    //printf("mother fucker" );
    exit (1);

}

void int_handler (int signum)
{
    kill(pid, SIGINT);
}

void exit_handler (void)
{
    //reset_input_mode();
}




