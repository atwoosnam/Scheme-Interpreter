#include "linkedlist.h"
#include "talloc.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

// Global variable to contain all the tokens
Value *list;

// Function that will take a char and determine if it's a digit.
// Note: We wrote our own function for this instead of using isdigit()
// because we encountered problems with isdigit running on mirage.
int isDigit(char charRead){
	char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};
	int i;
	for (i = 0; i < 10; i++) {
		if(charRead == numbers[i])
			return 1;
	}
	return 0;
}

// Deal with an open parenthesis
Value *readOpen() {
	Value *toAdd = (Value *)talloc(sizeof(Value));
	(*toAdd).type = OPEN_TYPE;
	return toAdd;
}

// Deal with a close parenthesis
Value *readClose() {
	Value *toAdd = (Value *)talloc(sizeof(Value));
	(*toAdd).type = CLOSE_TYPE;
	return toAdd;
}

// Deal with a boolean
Value *readBool() {
    Value *toAdd = (Value *)talloc(sizeof(Value));
	(*toAdd).type = BOOL_TYPE;
	char charRead = fgetc(stdin);
	if (charRead == 't')
		(*toAdd).i = 1;
	else if (charRead == 'f')
		(*toAdd).i = 0;
	charRead = fgetc(stdin);
	if (charRead != ' ' && charRead != '\n' && charRead != ')' \
		&& charRead != '(' && charRead != EOF) {
		printf("\nError: Bad boolean syntax\n");
		texit(0);
	}
	charRead = ungetc(charRead, stdin);
	return toAdd;
 }

// Deal with a string
 Value *readString() {
 	// Allocate memory for the token
    Value *toAdd = (Value *)talloc(sizeof(Value));
	(*toAdd).type = STR_TYPE;	

	// Allocate memory for the string
	char *strToken = (char *)talloc(sizeof(char)*1000);
	strToken[0] = '"';
	char charRead = fgetc(stdin);
	int index = 1;

	//checking for EOF to prevent infinte while loop
	while (charRead != '"' && charRead != EOF) { 
		strToken[index] = charRead;
		index++;
    	charRead = fgetc(stdin);
	}

	// Can't reach the EOF without finishing the string (reaching a close quote)
	if (charRead == EOF) {
		printf("\nError: Reached end of file without reading a close quote.\n");
		texit(0);
	}
	// Close quote. String is over.
	else if (charRead == '"') {
		strToken[index] = charRead;
		strToken[index+1] = '\0';
	}
	(*toAdd).s = (char *)talloc(1000 * sizeof(char));
	strcpy((*toAdd).s, strToken);
	return toAdd;
}

// Deal with a float
Value *readDotDigitFloat(int neg) {
	// neg is a parameter that is 1 if the number is negative
    Value *toAdd = (Value *)talloc(sizeof(Value));
	(*toAdd).type = DOUBLE_TYPE;
	(*toAdd).d = 0;
	char charRead = fgetc(stdin);
	double decimalIndex = .1;

	// Get the rest of the digits in the float
	while (isDigit(charRead)) {
		// Converting from char to int
		int digitChar = charRead - '0';
		(*toAdd).d += decimalIndex * digitChar;
		charRead = fgetc(stdin);
		decimalIndex = decimalIndex/10;
	}

	// Deal with a negative float
	if (neg == 1) {
		(*toAdd).d = - (*toAdd).d;
	}
	// Invalid char at the end of this float, hence not a valid float.
	if (charRead != ' ' && charRead != ')' && charRead != EOF && charRead == '\n' && charRead != '(') {
		printf("\nError: Not a valid float.\n");
		texit(0);
	} 
	// Valid float.
	else if (charRead == ' ' || charRead == EOF || charRead == '\n') {
		return toAdd;
	}
	// Encountered an open parenthesis. Add that to our tokens too.
	else if (charRead == '(') {
		list = cons(toAdd, list);
        Value *toAdd = (Value *)talloc(sizeof(Value));
        (*toAdd).type = OPEN_TYPE;
        return toAdd;	
	}
	// Encountered a close parenthesis. Add that to our tokens too.
	else {
	    list = cons(toAdd, list);
        Value *toAdd = (Value *)talloc(sizeof(Value));
        (*toAdd).type = CLOSE_TYPE;
        return toAdd;
	}
	// We need this to get rid of a warning.
	// We shouldn't ever actually return NULL, or ever see the comment below.
	printf("You should not be seeing this - readDotDigitFloat tokenizer.c\n");
	return NULL;

}

// Deal with integers
Value *readNumNoSign(char charRead, int neg) {

	// This function is called when we're starting with a digit
	Value *toAdd = (Value *)talloc(sizeof(Value));
	double num = charRead - '0';
	charRead = fgetc(stdin);

	// While we're seeing digits, do math to add them to the number
	while (isDigit(charRead) && charRead != '.') {
		int digitChar = charRead - '0';
		num = num * 10 + digitChar;
		charRead = fgetc(stdin);
	}

	// Invalid integer, we encountered a character that can't be in an int
	if (charRead != ' ' && charRead != '.' && charRead != ')'&& charRead != '(' && charRead != EOF && charRead != '\n') {
		printf("\nError: Not a valid int.\n");
		texit(0);
	}

	// If we see a space, we're done (and it's an int)
	 else if (charRead == ' ' || charRead == EOF || charRead == '\n') {
		(*toAdd).type = INT_TYPE;
		if (neg == 1)
			num = - num;
		(*toAdd).i = (int) num;
		return toAdd;
	}

	// If we see a ')', we're done (and it's an int), and we also need
	// to tokenize the ')'
	else if (charRead == ')') {
		(*toAdd).type = INT_TYPE;
		if (neg == 1) {
			num = - num;
		}
		(*toAdd).i = (int) num;
		list = cons(toAdd, list);
		Value *toAdd = (Value *)talloc(sizeof(Value));
        (*toAdd).type = CLOSE_TYPE;
        return toAdd;
	}
	// Same with '('
	else if (charRead == '(') {
		(*toAdd).type = INT_TYPE;
		if (neg == 1) {
			num = - num;
		}
		(*toAdd).i = (int) num;
		list = cons(toAdd, list);
		Value *toAdd = (Value *)talloc(sizeof(Value));
        (*toAdd).type = OPEN_TYPE;
        return toAdd;
	}

	// Character was '.', so we're after the decimal point
	// Do basically the same thing, but add it to what is now
	// a double instead of an int
	else {
		(*toAdd).type = DOUBLE_TYPE;
		double decimalIndex = .1;
		charRead = fgetc(stdin);

		while (isDigit(charRead)) {
			int digitChar = charRead - '0';
			num += decimalIndex * digitChar;
			decimalIndex = decimalIndex/10;
			charRead = fgetc(stdin);
		}

		// Invalid character encountered, hence we don't have a valid float.
		if (charRead != ' ' && charRead != ')'&& charRead != '(' && charRead != '\n' && charRead != EOF) {
			printf("Error: Not a valid float.\n");
			texit(0);
		}

		// We've gone through the entire float
		else if (charRead == ' ' || charRead == '\n' || charRead == EOF){
			if (neg == 1) {
				num = - num;
			}
			(*toAdd).d = num;
			return toAdd;
		} 

		// current char is '(' so deal with it
		else if (charRead == '(') {
			if (neg == 1)
				num = - num;
			(*toAdd).d = num;
			list = cons(toAdd,list);
			Value *toAdd = (Value *)talloc(sizeof(Value));
			(*toAdd).type = OPEN_TYPE;
			return toAdd;
		}
		// same with ')'
		else {
			if (neg == 1)
				num = - num;
			(*toAdd).d = num;
			list = cons(toAdd,list);
			Value *toAdd = (Value *)talloc(sizeof(Value));
			(*toAdd).type = CLOSE_TYPE;
			return toAdd;
		}
	}

	// We need this to get rid of a warning. We 
	// should never actually get here.
	printf("You should not be seeing this - readNumNoSign tokenizer.c\n");
	return NULL;
}

// Function to check for a valid start of a symbol
// Note that we don't use isalpha within this function since we encountered
// issues with it while running our program on mirage.
int isInitial(char charRead) {
	char initial[] = {'!','$','%','&','*','/',':','<','=','>','?','~','_','^','\0'};
	char letters[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p', \
	                  'q','r','s','t','u','v','w','x','y','z','A', 'B', 'C', 'D', 'E', \
	                  'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', \
	                  'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\0'};
	int i;
	for (i = 0; i < 53; i++) {
		if (letters[i] == charRead){
			return 1;
		}
	}
	for (i = 0; i < 15; i++) {
		if (initial[i] == charRead){
			return 1;
		}
	}
	return 0;
}

// Function that is used to check that the rest of a given 
// input is appropriate to be classified as a symbol
int isSubsequent(char charRead) {
	if (isInitial(charRead) == 1 || isDigit(charRead) == 1)
		return 1;
	if (charRead == '.' || charRead == '+' || charRead == '-')
		return 1;
	return 0;
}

// Deal with an identifier
Value *readSymbol(char charRead) {
	Value *toAdd = (Value *)talloc(sizeof(Value));
	(*toAdd).type = SYMBOL_TYPE;
	int size = 1000 * (sizeof(char));
	char *symToken = (char *)talloc(size);
	symToken[999] = '\0';

	int strIndex = 0;
	symToken[strIndex] = charRead;
	charRead = fgetc(stdin);

	// Iterate over input and read in the token
	while(isSubsequent(charRead)) {
		strIndex++;
		symToken[strIndex] = charRead;
		charRead = fgetc(stdin);
	}
	strIndex++;
	symToken[strIndex] = '\0';

	(*toAdd).s = (char *)talloc(1000 * sizeof(char));
	(*toAdd).s = symToken;

	// Invalid char encountered, so invalid identifier
	if (charRead != ' ' && charRead != ')' && charRead != '(' && charRead !='\n' && charRead != EOF) {
		printf("\nError: Not a valid identifier.\n");
		texit(0);
	} 

	// Valid identifier
	else if (charRead == ' ' || charRead == '\n' || charRead == EOF) {
		return toAdd;
	} 

	// Valid identifier but we need to account for an open paren
	else if (charRead == '(') {
	    list = cons(toAdd, list);
        Value *closeParen = (Value *)talloc(sizeof(Value));
        (*closeParen).type = OPEN_TYPE;
        return closeParen;
	}
	// Same with close paren
	else {
	    list = cons(toAdd, list);
        Value *closeParen = (Value *)talloc(sizeof(Value));
        (*closeParen).type = CLOSE_TYPE;
        return closeParen;
	}

	// We need this to get rid of a warning.
	// We should never actually get here.
	printf("You should not be seeing this - readSymbol tokenizer.c\n");
	return NULL;
}

// Deal with just a + or - symbol
void plusMinus(char charRead, int ifNeg) {
	Value *toAdd = (Value *)talloc(sizeof(Value));
	(*toAdd).type = SYMBOL_TYPE;
	char *str = (char *)talloc(10 * sizeof(char));
	if (ifNeg == 0)
		str[0] = '+';
	else
		str[0] = '-';
	(*toAdd).s = (char *)talloc(10 * sizeof(char));
	strcpy((*toAdd).s,str);
	list = cons(toAdd, list);

	// If we immediately encouter a close paren, deal with it
	if (charRead == ')') {
		Value *closeParen = (Value *)talloc(sizeof(Value));
		(*closeParen).type = CLOSE_TYPE;
		list = cons(closeParen, list);
	}
}

// Function that will take in input from a text file and
// tokenize it in a linked list.
Value *tokenize(){
	char charRead;
    list = makeNull();
    charRead = fgetc(stdin);

    while (charRead != EOF) {
        if (charRead == '(') {
            list = cons(readOpen(), list);
        }
        else if (charRead == ')') {
            list = cons(readClose(), list);
        }
        else if (charRead == '#') {
            list = cons(readBool(), list);
        }
        else if (charRead == '"') {
        	Value *toAdd = (Value *)talloc(sizeof(Value));
        	toAdd = readString();
            list = cons(toAdd, list);
        }
        else if (charRead == '.') {
        	list = cons(readDotDigitFloat(0),list);
        }
        // float or int
        else if (isDigit(charRead)) {
        	list = cons(readNumNoSign(charRead, 0), list);
        }
        // Comment. Tokenize should skip the rest of this line
        else if (charRead == ';') {
        	while(charRead != '\n' && charRead != EOF)
        		charRead = fgetc(stdin);
        }
        // could be number, character, or symbol
        else if (charRead == '+' || charRead == '-') {
        	int ifNeg = 0;
        	if (charRead == '-')
        		ifNeg = 1;

        	charRead = fgetc(stdin);
        	if (charRead == '.') {
        		list = cons(readDotDigitFloat(ifNeg), list);
        	}
        	else if (isDigit(charRead) == 1) {
        		list = cons(readNumNoSign(charRead, ifNeg), list);
    		}
    		else if (charRead == ' ' || charRead == ')' || charRead == EOF || charRead == '\n'){
    			plusMinus(charRead, ifNeg);
    		}
    		else {
        		printf("Symbols can't start with + or - \n");
        		texit(0);
        	}
        }
        // It's an identifier, dude
        else if (isInitial(charRead) == 1) {
        	list = cons(readSymbol(charRead), list);
        }
        charRead = fgetc(stdin);
    }
    Value *revList = reverse(list);
    return revList;
}

// Function that will display each value type appropriately
void displayTokensSwitch(Value *element) {
	switch ((*element).type) {
		case INT_TYPE:
		    printf("%d:integer\n",(*element).i);
		    break;
		case DOUBLE_TYPE:
		    printf("%f:float\n",(*element).d);
		    break;
		case STR_TYPE:
			assert(element);
			printf("%s:string\n",(*element).s);
			break;
		case BOOL_TYPE:
			if((*element).i == 0)
				printf("#f:boolean\n");
			else
				printf("#t:boolean\n");
			break;
		case SYMBOL_TYPE:
			printf("%s:symbol\n",(*element).s);
			break;
		case OPEN_TYPE:
			printf("(:open\n");
			break;
		case CLOSE_TYPE:
			printf("):close\n");
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

// Helper function for displayTokens
void displayTokensHelper(Value *list) {
	if ((*list).type == CONS_TYPE) {
		displayTokensSwitch((*list).c.car);
		displayTokensHelper((*list).c.cdr);
	}
}

// Function that will print out all tokens in the linked list
void displayTokens(Value *list) {
	assert(list);
	assert((*list).type == CONS_TYPE);
	displayTokensHelper(list);
}
