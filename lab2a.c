#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

int main (int argc, char* argv[])
{
	/*
		get the options
	*/
	int num_thread=1;
	int num_iteration=1;

	while (1)
    {
      static struct option long_options[] =
        {
          //set flag
          {"encrypt", no_argument,  &flag_encrypt, 1},
        
          
          //set value
          
          
          {"threads",  required_argument, 0, 't'},
          {"iterations",  required_argument, 0, 'i'},
          
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      arg = getopt_long (argc, argv, "ti",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (arg == -1)
        break;

      switch (arg)
        {
        
        case 0:
          break;


        case 't':
          num_thread = optarg;
          break;

        case 'i':
          num_iteration = optarg;
          break;

        default:
          return 0;


        }
    }

    printf('thread:%s\niteration:%s', num_thread, num_iteration);
}

