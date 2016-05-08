#define _GNU_SOURCE
#include "SortedList.h"
#include <string.h>
#include <pthread.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	SortedListElement_t *listptr = list;
	if (opt_yield & INSERT_YIELD)
		pthread_yield();
	while (listptr->next != list && (strcmp(listptr->next->key, element->key) < 0 ))
		listptr = listptr->next;
	element->prev = listptr;
	element->next = listptr->next;
	listptr->next = element;
	element->next->prev = element;
}

int SortedList_delete( SortedListElement_t *element)
{
	if (opt_yield & DELETE_YIELD)
		pthread_yield();
	if (element->next->prev != element || element->prev->next != element)
		return 1;
	element->next->prev = element->prev;
	element->prev->next = element->next;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	SortedListElement_t *listptr = list;
	if (opt_yield & SEARCH_YIELD)
		pthread_yield();
	while (listptr->next != list && strcmp(listptr->next->key, key))
		listptr = listptr->next;
	if (listptr->next == list)
		return NULL;
	return listptr->next;
}

int SortedList_length(SortedList_t *list)
{
	int length = 0;
	if (opt_yield & SEARCH_YIELD)
		pthread_yield();
	for (SortedListElement_t *listptr = list; listptr->next != list; listptr = listptr->next) {
		if (listptr->next->prev != listptr)
			return -1;
		length++;
	}
	return length;
}
