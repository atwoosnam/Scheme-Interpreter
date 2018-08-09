#include "talloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "linkedlist.h"

// Value *activeList = malloc(sizeof(Value));
Value *activeList;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size) {
	if (!activeList) {
		activeList = malloc(sizeof(Value));
		(*activeList).type = NULL_TYPE;
	}
	Value *newElement = malloc(size);
	Value *consValue = malloc(sizeof(Value));
	(*newElement).type = PTR_TYPE;
	(*consValue).type = CONS_TYPE;

	(*consValue).c.car = newElement;
	(*consValue).c.cdr = activeList;
	activeList = consValue;
	return newElement;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
	if (activeList) {
		Value *current = activeList;
		// next pointer = current pointer
		Value *next = current;
		while ((*current).type != NULL_TYPE) {
			next = cdr(current);
			// free the thing current is pointing to
			//printf("first \n");
			free(car(current));
			//printf("\n");
			// free the current pointer
			//printf("second \n");
			free(current);
			//printf("\n");
			current = next;
		}
		free(current);
		activeList = NULL;
	}
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
	tfree();
	exit(status);
}