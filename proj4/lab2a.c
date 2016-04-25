#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <pthread.h>
//#include <time.h>
#include <time.h>


//initialize a longlong counter to 0
long long counter = 0;
pthread_mutex_t count_mutex;
int opt_yield;
volatile int lock = 0;


void add(long long *pointer, long long value) 
{
    switch (sync)
    {
      case 'm':
        pthread_mutex_lock(&count_mutex);
        long long sum = *pointer + value;
        if (opt_yield)
        pthread_yield();
        *pointer = sum;
        pthread_mutex_unlock(&count_mutex);
      break;

      case 's':
        while (__sync_lock_test_and_set(&lock, 1));
        // critical section
        long long sum = *pointer + value;
        if (opt_yield)
        pthread_yield();
        *pointer = sum;
        __sync_lock_release(&lock);
      break;

      case 'c':
        while (__sync_val_compare_and_swap(&lock, 0, 1) == 1);
        long long sum = *pointer + value;
        if (opt_yield)
        pthread_yield();
        *pointer = sum;

      break;

      default:
        long long sum = *pointer + value;
        if (opt_yield)
        pthread_yield();
        *pointer = sum;
      break;

    }
}



void *thread_func(void *num_iteration)
{
   long numIteration;
   numIteration = (long)num_iteration;
   //add 1 for n times
   int i;
   for (i=0; i<numIteration; i++)
   {
      add(&counter, 1);
   }
   //subtract 1 for n times
   for (i=0; i<numIteration; i++)
   {
      add(&counter, -1);
   }

   pthread_exit(NULL);
}

/*
  calculate the diff of two timers
*/

struct timespec diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}




int main (int argc, char* argv[])
{
	/*
		get the options
	*/
	long num_thread=1;
	long num_iteration=1;
  void *status;

	while (1)
    {
      static struct option long_options[] =
        {
          //set flag
          {"yield", no_argument,  &opt_yield, 1},
        
          
          //set value
          
          
          {"threads",  required_argument, 0, 't'},
          {"iterations",  required_argument, 0, 'i'},
          
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      char arg = getopt_long (argc, argv, "ti",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (arg == -1)
        break;

      switch (arg)
        {
        
        case 0:
          break;


        case 't':
          num_thread = atoi(optarg);
          break;

        case 'i':
          num_iteration = atoi(optarg);
          break;

        //default:
          //return 0;


        }
    }

    //printf("thread:%ld\niteration:%ld", num_thread, num_iteration);

  /*
    set up the timer to keep tracking the time
    reference: https://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime
  */
  struct timespec time1, time2;

  /* start time*/
  clock_gettime(CLOCK_MONOTONIC, &time1);

  /*
    start the thread add and minus 1 to the counter
    reference: https://computing.llnl.gov/tutorials/pthreads/
  */
  pthread_t threads[num_thread];
   int rc;
   long t;
   for(t=0; t<num_thread; t++){
      //printf("In main: creating thread %ld\n", t);
      rc = pthread_create(&threads[t], NULL, thread_func, (void *)num_iteration);
      if (rc){
         //printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
   }
   /*
   join the threads
   */
   for(t=0; t<num_thread; t++) 
   {

      if (rc) 
      {
         printf("ERROR; return code from pthread_join()is %d\n", rc);
         exit(-1);
      }
      //printf("Main: completed join with thread %ld having a status of %ld\n",t,(long)status);
   }

  /* end time */
  clock_gettime(CLOCK_MONOTONIC, &time2);
  
  /*
    print out the log:
  */
  int elapsed_time = diff(time1, time2).tv_nsec;

  //print discription
  printf(" ./lab2a --iterations=%ld --threads=%ld\n%ld threads x %ld iterations x (add + subtract) = %ld operations\n", num_iteration, num_thread, num_thread, num_iteration, num_iteration*num_thread);
  if (counter!=0)
    printf("ERROR: final count = %lld\n", counter);
  printf("elapsed time: %dns\n", elapsed_time);
  printf("per operation: %ldns\n", elapsed_time/(num_iteration*num_thread));
  pthread_exit(NULL);
}

