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

int threads = 1;
int iterations = 1;
int operations = 1;
int opt_yield = 0;
char locktype = 'n';
int spinlock = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
SortedListElement_t *element;
SortedList_t list = {&list, &list, NULL};
char *keys;

void error(char *msg)
{
    fprintf(stderr, msg);
    exit(2);
}

void* thread_func(void* argc)
{
  for (int i = *(int *)argc; i < operations; i += threads) {
    switch (locktype) {
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

  switch (locktype) {
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

for (int i = *(int *)argc; i < operations; i += threads) {
    switch (locktype) {
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
    int c;
    char *yieldptr;    
    int yieldopt_count = 0;       
    while (1) {
    static struct option long_options[] =
    {
      {"threads", required_argument, 0, 't'},
      {"iterations", required_argument, 0, 'i'},
      {"yield", required_argument, 0, 'y'},
      {"sync", required_argument, 0, 's'}
    };
    int option_index = 0;
    c = getopt_long (argc, argv, "", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
      case 0:
        break;
      case 'y':
        for (yieldptr = (char *)optarg; *yieldptr != '\0' && yieldopt_count < 3; yieldptr++, yieldopt_count++) {
          if (*yieldptr == 'i')
            opt_yield += 1;
          else if (*yieldptr == 'd')
            opt_yield += 2;
          else if (*yieldptr == 's')
            opt_yield += 4;
          else
            error("invalid yield option\n");
        }
        break;
      case 't':
        threads = atoi(optarg);
        if (threads < 1)
          error("invalid thread number\n");
        break;
      case 'i':
        iterations = atoi(optarg);
        if (iterations < 1)
          error("invalid iteration number\n");
        break;
      case 's':
        locktype = *(char *)optarg;
        if (!(locktype == 'm' || locktype == 's'))
          error("invalid sync option\n");
        break;
      default:
        error("invalid option\n");
  } }
  
  operations = threads * iterations;
  element = (SortedListElement_t *)malloc(operations*sizeof(SortedListElement_t));
  if (element == NULL)
    error("malloc fail\n");
  keys = (char *)malloc(operations*6*sizeof(char));
  if (keys == NULL)
    error("malloc fail\n");
  srand(time(NULL));
  for (int i = 0; i < operations * 6; i += 6) {
    for (int j = 0; j < 5; j++)
      keys[i+j] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[rand() % 62];
    keys[i+5] = '\0';
  }

  for (int i = 0, j = 0; i < operations; i++, j += 6)
      element[i].key = &keys[j];

  pthread_t *tids = (pthread_t *)malloc(threads*sizeof(pthread_t));
  if (tids == NULL)
    error("malloc fail\n");

  int *tid_id = (int *)malloc(threads*sizeof(int));
  if (tid_id == NULL)
    error("malloc fail\n");
  for (int i = 0; i < threads; i++)
    tid_id[i] = i;

  struct timespec requestStart, requestEnd;
  if (clock_gettime(CLOCK_MONOTONIC, &requestStart))
    error("clock_gettime fail\n");

  for (int i = 0; i < threads; i++)
    pthread_create(&tids[i], NULL, thread_func, &tid_id[i]);

  for (int j = 0; j < threads; j++)
    pthread_join(tids[j], NULL);

  if (clock_gettime(CLOCK_MONOTONIC, &requestEnd))
    error("clock_gettime fail\n");

	if (locktype == 'm')
		pthread_mutex_destroy(&lock);

  free(tid_id);
  free(tids);
  free(keys);
  free(element);

  long long total_time = (requestEnd.tv_sec - requestStart.tv_sec) * BILLION
    + (requestEnd.tv_nsec - requestStart.tv_nsec);
  fprintf(stdout, "%d threads x %d iterations x (insert + lookup//delete) = %d operations\n", 
    threads, iterations, operations * 2);
  fprintf(stdout, "elapsed time: %lldns\n", total_time);
  fprintf(stdout, "per operation: %lldns\n", total_time / operations / 2);

  if (SortedList_length(&list)) {
    fprintf(stderr, "ERROR!\n");
    exit(1);
  }

  exit(0);
}
