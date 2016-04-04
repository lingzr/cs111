#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

void sig_handler(int signo)
 {
    	perror("Catched a segmentation fault");
    	exit(3);
 }

int main (int argc, char* argv[])
{
	int input_file = 0;
	int output_file = 1; //create input and output file descriptor
	ssize_t input_block, output_block; //number of size read.write by read() and write()
	char buffer [10000]; //the buffer used to store the read in char

	//declare variable for arguemnt
	static int flag_segfault ;
	static int flag_catch ;
	char* input_file_name = NULL;
	char* output_file_name = NULL;
	int arg;

	//parsing the argument
	
	while (1)
    {
      static struct option long_options[] =
        {
          //set flag
          {"segfault", no_argument,  &flag_segfault, 1},
          {"catch",   no_argument,   &flag_catch, 1},
          
          //set value
          
          {"input",  required_argument, 0, 'i'},
          {"output",    required_argument, 0, 'o'},
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

        case 'i':
          input_file_name = optarg;
          break;

        case 'o':
          output_file_name = optarg;
          break;

        default:
          return 0;


        }
    }

    /*
    	handle catch flag
    */

   

    if (flag_catch)
    {
    	signal(SIGSEGV, sig_handler);
    }

    /*
    	handle segfault flag
    */
    if (flag_segfault)
    {
    	//printf("i am here here here here");
    	char* dush = NULL;
    	*dush = 'a';

    }




	//crate input file
	if (input_file_name != NULL)
	{
		input_file = open(input_file_name, O_RDONLY);
		dup2(input_file, 0);
		if (input_file==-1 )
		{
			perror ("error opening the input file");
			return 1;
		}
	}
	

	//create the file descriptor for the output file

	if (output_file_name != NULL)
	{
		output_file = open(output_file_name, O_WRONLY | O_CREAT, 0600);
		dup2(output_file, 1);
		if (output_file==-1 )
		{
			perror("error opening the output file");
			return 2;
		}
	}
	

	//copy from the stdin to stdout
	while ((input_block = read (0, &buffer, 10000))>0)
	{
		output_block = write (1, &buffer, input_block);
		if (input_block != output_block)
		{
			perror ("error writing from stdin to stdout");
			return 4;
		}

	}

	//close file descriptor
	close(input_file);
	close(output_file);

	return 0;
}


 