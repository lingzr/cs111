#define _GNU_SOURCE
#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <time.h>



long num_thread=1;
long num_iteration=1;
int operations = 1;
int opt_yield = 0;
char* temperal;

int locker = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
SortedListElement_t *element;
SortedList_t list = {&list, &list, NULL};
char *keys;
//initialize a longlong counter to 0
long long counter = 0;
pthread_mutex_t count_mutex;

char sync_s;


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

void* thread_func(void* argc)
{
  int i;
  //insert 
  for (i = *(int*)argc; i < operations; i += num_thread) 
  {
    if (sync_s == 'm')
    {
      pthread_mutex_lock(&lock);
      SortedList_insert(&list, &element[i]);
      pthread_mutex_unlock(&lock);
    }
    else if (sync_s=='s')
    {
      while (__sync_lock_test_and_set(&locker, 1));
        SortedList_insert(&list, &element[i]);
        __sync_lock_release(&locker);
    }
    else
    {
      SortedList_insert(&list, &element[i]);
    }
  }
  //get the length
  if (sync_s=='m')
  {
    pthread_mutex_lock(&lock);
    SortedList_length(&list);
    pthread_mutex_unlock(&lock);
  }
  else if (sync_s=='s')
  {
    while (__sync_lock_test_and_set(&locker, 1));
    SortedList_length(&list);
    __sync_lock_release(&locker);
  }
  else
  {
    SortedList_length(&list);
  }

  //lookup and delete.
  SortedListElement_t* node_deleted;
  for ( i = *(int *)argc; i < operations; i += num_thread) 
    {

      if (sync_s=='m')
      {
        pthread_mutex_lock(&lock);
        node_deleted = SortedList_lookup(&list, element[i].key);
        SortedList_delete(node_deleted);
        pthread_mutex_unlock(&lock);
      } 
      else if (sync_s=='s')
      {
        while (__sync_lock_test_and_set(&locker, 1));
        node_deleted = SortedList_lookup(&list, element[i].key);
        SortedList_delete(node_deleted);
        __sync_lock_release(&locker);
      }
      else
      {
        node_deleted = SortedList_lookup(&list, element[i].key);
        printf("%s\n", node_deleted->key);
        SortedList_delete(node_deleted);
      }
    } 
}

int main(int argc, char *argv[])
{
    int i, j;
  /*
    get the options
  */

  while (1)
    {
      static struct option long_options[] =
        {
          //set value
          {"threads",  required_argument, 0, 't'},
          {"yield",  required_argument, 0, 'y'},
          {"iterations",  required_argument, 0, 'i'},
          {"sync",  required_argument, 0, 's'},
          
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      char arg = getopt_long (argc, argv, "tis",
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

        case 's':
          sync_s = optarg[0];
          break;

        case 'y':
          temperal = optarg;
          int i=0;
          for (i=0; temperal[i]!= '\0'; i++)
          {
            if (temperal[i]=='i')
            {
              opt_yield+=INSERT_YIELD;
            }
            else if (temperal[i]=='d')
            {
              opt_yield+=DELETE_YIELD;
            }
            else if (temperal[i]=='s')
            {
              opt_yield+=SEARCH_YIELD;
            }
            else
            {
              fprintf(stderr, "invalid option!\n");
            }

          }
          break;

        //default:
          //return 0;


        }
    }
  
  //initialize all the node needed
  operations = num_thread * num_iteration;
  element = (SortedListElement_t *)malloc(operations*sizeof(SortedListElement_t));


  //generate key for each node
  for (i = 0; i < operations; i++)
  {
    char* temp = (char*) malloc (5*sizeof(char));
    
    temp[0]=65+rand()%60;
    temp[1]=65+rand()%60;
    temp[2]=65+rand()%60;
    temp[3]=65+rand()%60;
    temp[4]=65+rand()%60;
    element[i].key = temp;

    // element[i].key = (char*) malloc (5*sizeof(char));
    // element[i].key[0] = 65+rand()%60;
    // element[i].key[1] = 65+rand()%60;
    // element[i].key[2] = 65+rand()%60;
    // element[i].key[3] = 65+rand()%60;
    // element[i].key[4] = 65+rand()%60;



  }

  pthread_t *tids = (pthread_t *)malloc(num_thread*sizeof(pthread_t));
 


  struct timespec requestStart, requestEnd;
  clock_gettime(CLOCK_MONOTONIC, &requestStart);
    
 

  //an integer array to hold ids
  int *thread_id = (int *)malloc(num_thread*sizeof(int));
 
  for ( i = 0; i < num_thread; i++)
    thread_id[i] = i;

  int k;
  for ( k = 0; k < num_thread; k++)
    pthread_create(&tids[k], NULL, thread_func, &thread_id[k] );

  
  for ( j = 0; j < num_thread; j++)
    pthread_join(tids[j], NULL);

  clock_gettime(CLOCK_MONOTONIC, &requestEnd);
   


  int elapsed_time = diff(requestStart, requestEnd).tv_nsec;

  //print discription
  printf("%ld threads x %ld iterations x (insert + lookup/delete) = %ld operations\n",  num_thread, num_iteration, operations*2);
  printf("elapsed time: %dns\n", elapsed_time);
  printf("per operation: %ldns\n", elapsed_time/operations/2);
  if (SortedList_length(&list)!=0)
  fprintf(stderr,"ERROR: final count = %lld\n", counter);


  exit(0);
}
