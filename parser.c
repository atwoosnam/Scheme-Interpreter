#include "parser.h"
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Function to display error when there are too many parentheses
// frees memory and exits program
void syntaxErrorA() {
	printf("Syntax error: too many close parentheses. \n");
	texit(1);
}

// Function to display error when there are too few parentheses
// frees memory and exits program
void syntaxErrorB() {
	printf("Syntax error: not enough close parentheses. \n");
	texit(1);

}

// Function that adds a token to our parse tree. Keeps track of
// how deep we are in the parse tree with a depth variable.
// Creates nested tree when we encounter a close paren
Value *addToParseTree(Value *tree, int *depth, Value *token) {
	
	// Update depth var if we see a paren
	if ((*token).type == OPEN_TYPE)
		(*depth)++;	
	else if ((*token).type == CLOSE_TYPE)
		(*depth)--;

	// Push anything that isn't a close paren
	if ((*token).type != CLOSE_TYPE) {
		tree = cons(token, tree);
	}
	// Just saw a close parenthesis
	else {
		Value *addList = makeNull();
		if ((*(*tree).c.car).type == OPEN_TYPE){
			addList = cons(makeNull(), addList);
		}
		while((*(*tree).c.car).type != OPEN_TYPE) {
			addList = cons((*tree).c.car, addList);
			tree = cdr(tree);
			if ((*tree).type == NULL_TYPE) {
				syntaxErrorA();
			}
		}
		// Pop the left parenthesis
		tree = cdr(tree);
		tree = cons(addList,tree);
	}
	return tree;
}

Value *parse(Value *tokens) {

	// init tree
    Value *tree = makeNull();
    // init depth var
    int depth = 0;

    Value *current = tokens;

    // The following block surrounds the tokens with
    // an open and close parenthesis so as to simulate a
    // list of tokens.
    Value *start = makeNull();
   	(*start).type = OPEN_TYPE;
   	Value *close = makeNull();
   	(*close).type = CLOSE_TYPE;
   	current = cons(start, current);
   	current = reverse(current);
   	current = cons(close, current);
   	current = reverse(current);

    assert(current != NULL && "Error (parse): null pointer");

    // While we have more tokens, add to parse tree
    while ((*current).type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree,&depth,token);
        current = cdr(current);
    }

    // We got through all of our tokens without seeing enough close
    // parentheses
    if (depth != 0) {
        syntaxErrorB();
    }

    // Get rid of the outer parentheses
    tree = car(tree);
    return tree;
}

// Function that will display each value type appropriately
void printTreeSwitch(Value *element) {
	switch ((*element).type) {
		case INT_TYPE:
		    printf("%d ",(*element).i);
		    break;
		case DOUBLE_TYPE:
		    printf("%f ",(*element).d);
		    break;
		case STR_TYPE:
			assert(element);
			printf("%s ",(*element).s);
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
		case OPEN_TYPE:
			break;
		case CLOSE_TYPE:
			break;
		case NULL_TYPE:
			break;
		case PTR_TYPE:
			break;
		case CONS_TYPE:
			break;
		case VOID_TYPE:
			break;
		case CLOSURE_TYPE:
			break;
		case PRIMITIVE_TYPE:
			break;
	}
}

// Helper function for printTree
void printTreeHelper(Value *tree) {
	if ((*tree).type == CONS_TYPE) {
		if ((*(*tree).c.car).type != CONS_TYPE) {
			printTreeSwitch((*tree).c.car);
		}
		else {
			printf("(");
			printTreeHelper((*tree).c.car);
			printf(")");
		}
		printTreeHelper((*tree).c.cdr);
	}
}


void printTree(Value *tree) {
	assert(tree);
	assert((*tree).type == CONS_TYPE);
	printTreeHelper(tree);
}