#include "linkedlist.h"
#include "talloc.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Create a new NULL_TYPE value node.
Value *makeNull() {
	Value *nullValue = talloc(sizeof(Value));
	(*nullValue).type = NULL_TYPE;
	return nullValue;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr) {
	assert(car);
	assert(cdr);
	Value *consValue = talloc(sizeof(Value));
	(*consValue).type = CONS_TYPE;

	(*consValue).c.car = car;
	(*consValue).c.cdr = cdr;

	return consValue;
}

// We have this here so we can call displayHelper within
// displaySwitch, so nested lists work.s
// We would have put it in the linkedlist.h, but we
// aren't allowed to modify it.
void displayHelper(Value *list);

// Determine which type and print accordingly
void displaySwitch(Value *element) {
	switch ((*element).type) {
		case INT_TYPE:
		    printf("%d ",(*element).i);
		    break;
		case DOUBLE_TYPE:
		    printf("%f ",(*element).d);
		    break;
		case STR_TYPE:
			printf("%s ",(*element).s);
			break;
		case CONS_TYPE:
			printf("(");
			displayHelper(element);
			printf(")");
			break;
		case PTR_TYPE:
		    printf("%p ",(*element).p);
		    break;
		case NULL_TYPE:
			break;
		case OPEN_TYPE:
			printf("(");
			break;
		case CLOSE_TYPE:
			printf(") ");
			break;
		case BOOL_TYPE:
			if((*element).i == 0)
				printf("#f ");
			else
				printf("#t ");
			break;
		case SYMBOL_TYPE:
			printf("%s ",(*element).s);
			break;
		case VOID_TYPE:
			break;
		case CLOSURE_TYPE:
			break;
		case PRIMITIVE_TYPE:
			break;
	}
}

// Called by display so that we can call it recursively
void displayHelper(Value *list) {
	if ((*list).type == CONS_TYPE){
		displaySwitch((*list).c.car);
		displayHelper((*list).c.cdr);
	} else if ((*list).type != NULL_TYPE){
		printf(". ");
		displaySwitch(list);
		// we're done because no more cons cells
	}
		// we're done because we reached NULL_TYPE
}

// Display the contents of the linked list to the screen in some kind of
// readable format
// Separate from helper function so we can assert that the first item
// is a cons cell, without worrying about the other items
void display(Value *list) {
	assert(list);
	assert((*list).type == CONS_TYPE || (*list).type == NULL_TYPE);
	if((*list).type == CONS_TYPE){
		printf("(");
		displayHelper(list);
		printf(")\n");
	}
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list) {
	assert(list);
	Value *newList = makeNull();
	while ((*list).type != NULL_TYPE) 
	{
		newList = cons((*list).c.car, newList);
		list = (*list).c.cdr;
	}
	return newList;

}

// Frees up all memory directly or indirectly referred to by list. Note that
// this linked list might consist of linked lists as items, so you'll need to
// clean them up as well.
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
void cleanup(Value *list) {
	assert(list);
	if ((*list).type == CONS_TYPE) {
		cleanup((*list).c.car);
		cleanup((*list).c.cdr);
		free(list);
		list = NULL;
	} else {
		free(list);
		list = NULL;
	}
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list) {
	assert(list);
	assert((*list).type == CONS_TYPE);
	return (*list).c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
	assert(list);
	assert((*list).type == CONS_TYPE);
	return (*list).c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
	// assert that 'value' is not a NULL POINTER (i.e. 'value's value != NULL)
	assert(value);
	// assert that value's type isn't something it's not supposed to be
	assert((*value).type < 5);
	assert((*value).type >= 0);
	if ((*value).type == NULL_TYPE) {
		return 1;
	}
	return 0;
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value) {
	assert(value);
	int counter = 0;
	while ((*value).type != NULL_TYPE)
	{
		counter++;
		value = (*value).c.cdr;
	}
	return counter;
}
