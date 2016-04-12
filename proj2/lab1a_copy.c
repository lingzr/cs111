#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <pthread.h>




// void reset_input_mode (void){
// 	tcsetattr (STDIN_FILENO, TCSANOW, &tattr);
// }

//this thread reads from shell and output to the stdout
void* thread_func (void *fd){
	char buf[128];
	int size = 0;

	while ((size = read(*(int *)fd, buf, 1))>0){
	//upon receiving EOF from the shell
	if (buf[0]==EOF){
		//restore the terminal mode and end;
		exit(1);
	}
	else{
		write (1, buf, size);
	}
					
	}


}


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

    //have the --shell flag
    if (flag_shell)
    {

		int pipe_from_shell[2]; //filedexcriptor readend[0] filedescriptorforwriteend[1]
		int pipe_to_shell [2];

		pipe (pipe_from_shell);
		pipe (pipe_to_shell);

		int pid = fork();

		if (pid==0){ //shell process

			close (pipe_from_shell[0]);//close the read end pipe from shell
			dup2(pipe_from_shell[1],1);

			close(pipe_to_shell[1]);
			dup2(pipe_to_shell[0], 0);

			execl("/bin/bash", "/bin/bash", NULL);

		}

		else{ //terminal proess
			//setup SIGPIPE handler using 'signal'
			//signal (SIGPIPE, handler);
			//terminal does not use the eandofpipe_to_shell
			close (pipe_to_shell[0]);
			//terminal does not use writeend of pipe from shell pipe(pipefrom_shell)
			close (pipe_from_shell[1]);

			//create thread variable
			pthread_t thread1;

			pthread_create(&thread1, NULL, &thread_func, &pipe_from_shell[0]);

			//main thread, read from keyboard and write to stdout and forward to shell
			char buf[1218];
			int size = 0;

			while ((size = read(0, buf, 128))>0){
				write (1, buf, size);
				write (pipe_to_shell[1], buf, size);
			}

    	}


	}

    
    //without --shell flag
    else
    {

    	//saved attribute
		struct termios tattr;
    	/* save the terminal attribute */
    	tcgetattr (STDIN_FILENO, &tattr);
    	//atexit(reset_input_mode);


    	//set the terminal mode
    	tcgetattr (0, &tattr);


		
		tattr.c_lflag &= ~(ICANON|ECHO);

		tattr.c_cc[VTIME]=0;
		tattr.c_cc[VMIN] = 1;

		tcsetattr(0, TCSAFLUSH, &tattr);

		char buf[1218];
		int size = 0;

		while ((size = read(0, buf, 128))>0){
			write (1, buf, size);
		}

    }
}







