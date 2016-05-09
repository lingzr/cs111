#define _GNU_SOURCE
#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define BILLION 1000000000

int num_thread = 1;
int num_iteration = 1;
int operations = 1;
int opt_yield = 0;

int spinlock = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
SortedListElement_t *element;
SortedList_t list = {&list, &list, NULL};
char *keys;
//initialize a longlong counter to 0
long long counter = 0;
pthread_mutex_t count_mutex;

char sync_s;
// volatile int lock = 0;


void error(char *msg)
{
    fprintf(stderr, msg);
    exit(2);
}

void* thread_func(void* argc)
{
  int i;
  for (i = *(int *)argc; i < operations; i += num_thread) {
    switch (sync_s) {
      case 'm':
        pthread_mutex_lock(&lock);
        SortedList_insert(&list, &element[i]);
        pthread_mutex_unlock(&lock);
        break;
      case 's':
        while (__sync_lock_test_and_set(&spinlock, 1))
          ;
        SortedList_insert(&list, &element[i]);
        __sync_lock_release(&spinlock);
        break;
      default:
        SortedList_insert(&list, &element[i]);
  } }

  switch (sync_s) {
      case 'm':
        pthread_mutex_lock(&lock);
        SortedList_length(&list);
        pthread_mutex_unlock(&lock);
        break;
      case 's':
        while (__sync_lock_test_and_set(&spinlock, 1))
          ;
        SortedList_length(&list);
        __sync_lock_release(&spinlock);
        break;
      default:
        SortedList_length(&list);
  }

for ( i = *(int *)argc; i < operations; i += num_thread) {
    switch (sync_s) {
      case 'm':
        pthread_mutex_lock(&lock);
        SortedList_delete(SortedList_lookup(&list, element[i].key));
        pthread_mutex_unlock(&lock);
        break;
      case 's':
        while (__sync_lock_test_and_set(&spinlock, 1))
          ;
        SortedList_delete(SortedList_lookup(&list, element[i].key));
        __sync_lock_release(&spinlock);
        break;
      default:
        SortedList_delete(SortedList_lookup(&list, element[i].key));
} } }

int main(int argc, char *argv[])
{
    int i, j;
    int c;
    char *yieldptr;    
    int yieldopt_count = 0;       
//     while (1) {
//     static struct option long_options[] =
//     {
//       {"num_thread", required_argument, 0, 't'},
//       {"num_iteration", required_argument, 0, 'i'},
//       {"yield", required_argument, 0, 'y'},
//       {"sync", required_argument, 0, 's'}
//     };
//     int option_index = 0;
//     c = getopt_long (argc, argv, "", long_options, &option_index);
//     if (c == -1)
//       break;
//     switch (c) {
//       case 0:
//         break;
//       case 'y':
//         for (yieldptr = (char *)optarg; *yieldptr != '\0' && yieldopt_count < 3; yieldptr++, yieldopt_count++) {
//           if (*yieldptr == 'i')
//             opt_yield += 1;
//           else if (*yieldptr == 'd')
//             opt_yield += 2;
//           else if (*yieldptr == 's')
//             opt_yield += 4;
//           else
//             error("invalid yield option\n");
//         }
//         break;
//       case 't':
//         num_thread = atoi(optarg);
//         if (num_thread < 1)
//           error("invalid thread number\n");
//         break;
//       case 'i':
//         num_iteration = atoi(optarg);
//         if (num_iteration < 1)
//           error("invalid iteration number\n");
//         break;
//       case 's':
//         locktype = *(char *)optarg;
//         if (!(locktype == 'm' || locktype == 's'))
//           error("invalid sync option\n");
//         break;
//       default:
//         error("invalid option\n");
//   } 
// }
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
          //set value
          {"num_thread",  required_argument, 0, 't'},
          {"yield",  required_argument, 0, 'y'},
          {"num_iteration",  required_argument, 0, 'i'},
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
          char* temp = optarg;
          int i=0;
          for (i=0; temp[i]!= '\0'; i++)
          {
            if (temp[i]=='i')
            {
              opt_yield+=INSERT_YIELD;
            }
            else if (temp[i]=='d')
            {
              opt_yield+=DELETE_YIELD;
            }
            else if (temp[i]=='s')
            {
              opt_yield+=SEARCH_YIELD;
            }

          }
          break;

        //default:
          //return 0;


        }
    }
  
  operations = num_thread * num_iteration;
  element = (SortedListElement_t *)malloc(operations*sizeof(SortedListElement_t));
  if (element == NULL)
    error("malloc fail\n");
  keys = (char *)malloc(operations*6*sizeof(char));
  if (keys == NULL)
    error("malloc fail\n");
  srand(time(NULL));
  for (i = 0; i < operations * 6; i += 6) {
    
    for ( j = 0; j < 5; j++)
      keys[i+j] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[rand() % 62];
    keys[i+5] = '\0';
  }
  
  for (i = 0, j = 0; i < operations; i++, j += 6)
      element[i].key = &keys[j];

  pthread_t *tids = (pthread_t *)malloc(num_thread*sizeof(pthread_t));
  if (tids == NULL)
    error("malloc fail\n");

  int *tid_id = (int *)malloc(num_thread*sizeof(int));
  if (tid_id == NULL)
    error("malloc fail\n");
  for ( i = 0; i < num_thread; i++)
    tid_id[i] = i;

  struct timespec requestStart, requestEnd;
  if (clock_gettime(CLOCK_MONOTONIC, &requestStart))
    error("clock_gettime fail\n");

  for ( i = 0; i < num_thread; i++)
    pthread_create(&tids[i], NULL, thread_func, &tid_id[i]);

  
  for ( j = 0; j < num_thread; j++)
    pthread_join(tids[j], NULL);

  if (clock_gettime(CLOCK_MONOTONIC, &requestEnd))
    error("clock_gettime fail\n");

	if (sync_s == 'm')
		pthread_mutex_destroy(&lock);

  free(tid_id);
  free(tids);
  free(keys);
  free(element);

  long long total_time = (requestEnd.tv_sec - requestStart.tv_sec) * BILLION
    + (requestEnd.tv_nsec - requestStart.tv_nsec);
  fprintf(stdout, "%d num_thread x %d num_iteration x (insert + lookup//delete) = %d operations\n", 
    num_thread, num_iteration, operations * 2);
  fprintf(stdout, "elapsed time: %lldns\n", total_time);
  fprintf(stdout, "per operation: %lldns\n", total_time / operations / 2);

  if (SortedList_length(&list)) {
    fprintf(stderr, "ERROR!\n");
    exit(1);
  }

  exit(0);
}
