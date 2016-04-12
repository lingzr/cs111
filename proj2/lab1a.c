#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>



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





int main (int argc, char* argv[])
{
	//parsing the argument
	int arg;
	static int flag_shell;
	while (1)
    {
      static struct option long_options[] =
        {
          //set flag
          {"shell", no_argument,  &flag_shell, 1},
          
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      arg = getopt_long (argc, argv, "io",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (arg == -1)
        break;

      switch (arg)
        {
        
        case 0:
          break;

        default:
          return 0;

		}
    }


    	
    set_input_mode();


    //have the --shell flag
    if (flag_shell)
    {
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

			//create thread variable
			pthread_t thread1;

			pthread_create(&thread1, NULL, &thread_func, &pipe_from_shell[0]);

			//main thread, read from keyboard and write to stdout and forward to shell
			char buf[128];
			int size = 0;



			while ((size = read(0, buf, 128))>0)
			{
				write (1, buf, size);
				write (pipe_to_shell[1], buf, size);
				if (buf[0]==4)
				{
					//reach the edn of the file, call the EOF_handler
					//printf("mother fucker!!\n");
					close (pipe_to_shell[1]);
					close (pipe_from_shell[0]);
					kill(pid, SIGHUP);
					reset_input_mode();
					exit (0);
				}
				if (buf[0] ==13 || buf[0] ==10)
				{
					int char13 =13;
					int char10 =10;
					write(1, &char13, size);
					write(1, &char10, size);
					write (pipe_to_shell[1], &char10, size);

				}
			}


    	}


	}

    
    //without --shell flag
    else
    {

		char buf[1218];
		int size = 0;

		while ((size = read(0, buf, 128))>0){
			write (1, buf, size);
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
		exit(1);
	}
	else{
		write (1, buf, size);
	}
					
	}


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
	reset_input_mode();
	//printf("mother fucker!" );
	exit (1);

}

void int_handler (int signum)
{
	kill(pid, SIGINT);
}

void exit_handler (void)
{
	int status;
	waitpid (pid, &status, 0);
	//if (WIFEXITED(status))
	printf("exit with status: %d\n", status );
	reset_input_mode();
}




