#include "SortedList.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>



/*
	struct SortedListElement 
	{
		struct SortedListElement *prev;
		struct SortedListElement *next;
		const char *key;
	};
*/
/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 *
 * Note: if (opt_yield & INSERT_YIELD)
 *		call pthread_yield in middle of critical section
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	//current points to the first node
	//cureent is used to trace the list
	SortedList_t* current = list->next;
	if (opt_yield & INSERT_YIELD)
		pthread_yield();
	//make the current points to the first element whose key is bigger or equal to the element.
	while (current!= NULL && current->key < element->key)
	{
		if (current->next == NULL)
			break;
		current = current->next;
	}
	//insert the element into the list
	//at the end of the list
	if (current->next == NULL)
	{
		element->prev = current;
		element->next = NULL;
		current->next = element;
	}
	//somewhere in the middle of the list
	else
	{
		element->prev = current->prev;
		element->next = current;
		current->prev->next = element;
		current->prev = element;
	}
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 * Note: if (opt_yield & DELETE_YIELD)
 *		call pthread_yield in middle of critical section
 */
int SortedList_delete( SortedListElement_t *element)
{
	//current points to the first node
	//cureent is used to trace the list
	SortedList_t* current = list->next;
	//make the current points to the first element whose key is equal to the element.
	if (opt_yield & DELETE_YIELD)
		pthread_yield();

	while (current!= NULL && current->key != element->key)
	{
		current = current->next;
	}
	//if we find the element
	if (current != NULL)
	{
		current->prev->next = current->next;
		current->next->prev = current->prev;
		current->prev = current;
		current->next = current;
		free(current);
	}
	return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 *
 * Note: if (opt_yield & SEARCH_YIELD)
 *		call pthread_yield in middle of critical section
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	//current points to the first node
	//cureent is used to trace the list
	SortedList_t* current = list->next;
	if (opt_yield & SEARCH_YIELD)
		pthread_yield();
	//make the current points to the first element whose key is equal to the element.
	while (current!= NULL && current->key != element->key)
	{
		current = current->next;
	}
	return current;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 *
 * Note: if (opt_yield & SEARCH_YIELD)
 *		call pthread_yield in middle of critical section
 */
int SortedList_length(SortedList_t *list)
{
	//current points to the first node
	//cureent is used to trace the list
	SortedList_t* current = list->next;
	if (opt_yield & SEARCH_YIELD)
		pthread_yield();
	int num_elements = 0;
	//make the current points to the first element whose key is equal to the element.
	while (current!= NULL)
	{
		num_elements++;
		current = current->next;
	}
	return num_elements;	
}

int main ()
{
	//create a head
	SortedList_t head;
	//create new element
	SortedListElement new_element;
	new_element->key = 's';
	SortedList_insert(head, new_element);
 
}





