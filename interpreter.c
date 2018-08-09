#include "value.h"
#include "interpreter.h"
#include "parser.h"
#include "linkedlist.h"
#include "talloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Global var to turn troubleshooting comments on/off:
bool troubleshooting = 0;

void flag(char* msg){
    if (troubleshooting) {
        printf("%s", msg);
    }
}

// Determine which type and print accordingly
void displayResult(Value *element) {
    switch ((*element).type) {
        case INT_TYPE:
            printf("%d\n",(*element).i);
            break;
        case DOUBLE_TYPE:
            printf("%f\n",(*element).d);
            break;
        case STR_TYPE:
            printf("%s\n",(*element).s);
            break;
        case CONS_TYPE:
            display(element);
            break;
        case PTR_TYPE:
            printf("%p\n",(*element).p);
            break;
        case NULL_TYPE:
            printf("()\n");
        case OPEN_TYPE:
            break;
        case CLOSE_TYPE:
            break;
        case BOOL_TYPE:
            if((*element).i == 0)
                printf("#f\n");
            else
                printf("#t\n");
            break;
        case SYMBOL_TYPE:
            printf("%s\n",(*element).s);
            break;
        case VOID_TYPE:
            break;
        case CLOSURE_TYPE:
            printf("#<procedure>\n");
            break;
        case PRIMITIVE_TYPE:
            printf("#<procedure>\n");
            break;
    }
}

// Function that prints appropriate evaluation error, and texits program
void evaluationError(int type) {
    if (type == 0)
        printf("Evaluation error: not a recognized special form.\n");
    else if (type == 1)
        printf("Evaluation error: invalid 'if' statement.\n");
    else if (type == 2)
        printf("Evaluation error: invalid 'let' statement.\n");
    else if (type == 3)
        printf("Evaluation error: Using an undefined variable.\n");
    else if (type == 4)
        printf("Evaluation error\n");
    else if (type == 5)
        printf("Evaluation error: empty file\n");
    else if (type == 6)
        printf("Evaluation error: bad syntax for 'define'.\n");
    else if (type == 7)
        printf("Evaluation error: bad parameters for 'lambda'.\n");
    else if (type == 8)
        printf("Evaluation error: bad function body for 'lambda'.\n");
    else if (type == 9)
        printf("Evaluation error: bad syntax for 'lambda'.\n");
    else if (type == 10)
        printf("Evaluation error: Number of parameters doesn't match number of function arguments.\n");
    else if (type == 11)
        printf("Evaluation error: No function associated with this identifier\n");
    else if (type == 12)
        printf("Evaluation error: Parameter type doesn't match function argument type\n");
    else if (type == 13)
        printf("Evaluation error: bad syntax for 'cons'. \n");
    else if (type == 14)
        printf("Evaluation error: can't take cdr of empty list \n");
    else if (type == 15)
        printf("Evaluation error: can't take car of empty list \n");
    else if (type == 16)
        printf("Evaluation error: bad syntax for 'and' \n");
    else if (type == 17)
        printf("Evaluation error: bad syntax for 'or' \n");
    else if (type == 18)
        printf("Evaluation error: not a procedure \n");
    else if (type == 19)
        printf("Evaluation error: invalid 'let*' statement.\n");
    else if (type == 20)
        printf("Evaluation error: invalid 'letrec' statement.\n");
    texit(1);
}
// Function to throw an evaluation error for undefined variables
void evaluationErrorUndefVar(char *var) {
    printf("Evaluation error: Using an undefined variable: %s\n", var);
    texit(1);
}

// Add primitive functions to top-level bindings list
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    Value *value = talloc(sizeof(Value));
    (*value).type = PRIMITIVE_TYPE;
    (*value).pf = function;
    Value *primName = makeNull();
    (*primName).type = SYMBOL_TYPE;
    (*primName).s = name;
    Value *bindings = reverse((*frame).bindings);
    bindings = cons(primName, bindings);
    bindings = cons(value, bindings);
    bindings = reverse(bindings);
    (*frame).bindings = bindings;    
}

// Helper function to check that args has
// two arguments
int validArgs(Value *args) {
    int elements = 0;
    Value *cur = args;
    while((*cur).type != NULL_TYPE) {
        elements++;
        cur = cdr(cur);
    }
    if (elements == 2)
        return 1;
    return 0;
}

// Deal with the car primitive
Value *primCar(Value *args) {
    if ((*cdr(args)).type != NULL_TYPE)
        evaluationError(10);
    if ((*car(args)).type != CONS_TYPE)
        evaluationError(12);
    if ((*car(car(args))).type == NULL_TYPE)
        evaluationError(15);
    return car(car(args));
}

// Deal with the cdr primitive
Value *primCdr(Value *args) {
    if ((*cdr(args)).type != NULL_TYPE)
        evaluationError(10);
    if ((*car(args)).type != CONS_TYPE)
        evaluationError(12);
    if ((*car(car(args))).type == NULL_TYPE)
        evaluationError(14);
    return cdr(car(args));
}

// Deal with the null? primitive
Value *primNull(Value *args) {
    Value *result = makeNull();
    (*result).type = BOOL_TYPE;
    if ((*cdr(args)).type != NULL_TYPE) {
        evaluationError(10);
    }
    if ((*car(args)).type == NULL_TYPE)
        (*result).i = 1;
    else if (car(car(args)) != NULL) {
        if ((*car(car(args))).type == NULL_TYPE) {
            (*result).i = 1;
        }
    }
    else
        (*result).i = 0; 
    return result;
}

// Deal with the mod operator. Note: doesn't work like
// mod in scheme right now.
Value *primMod(Value *args) {
    if (!validArgs(args))
        evaluationError(4);

    Value *result = makeNull();
    (*result).type = DOUBLE_TYPE;
    (*result).d = 0;
    Value *first = car(args);
    Value *second = car(cdr(args));

    if ((*first).type != INT_TYPE || (*second).type != INT_TYPE)
        evaluationError(4);

    (*result).d = (*first).i % (*second).i;
    return result;
}

// Deal with the + primitive
Value *primAdd(Value *args) {
    Value *result = makeNull();
    (*result).type = DOUBLE_TYPE;
    (*result).d = 0;
    Value *currNum;
    while ((*args).type != NULL_TYPE) {
        currNum = car(args);
        if ((*currNum).type == DOUBLE_TYPE) {
            (*result).d += (*currNum).d;
        }
        else if ((*currNum).type == INT_TYPE) {
            (*result).d += (*currNum).i;
        }
        else {
            evaluationError(12);
        }
        args = cdr(args);
    }
    return result;
}

Value *primMult(Value *args) {

    Value *result = makeNull();
    (*result).type = DOUBLE_TYPE;
    (*result).d = 1;
    Value *currNum;
    while((*args).type != NULL_TYPE) {
        currNum = car(args);
        if ((*currNum).type == DOUBLE_TYPE) {
            (*result).d *= (*currNum).d;
        }
        else if ((*currNum).type == INT_TYPE) {
            (*result).d *= (*currNum).i;
        }
        else {
            evaluationError(4);
        }
        args = cdr(args);
    }
    return result;
}

// Deal with the - primitive
Value *primSub(Value *args) {

    if (!validArgs(args))
        evaluationError(4);

    Value *result = makeNull();
    (*result).type = DOUBLE_TYPE;
    (*result).d = 0;
    Value *first = car(args);
    Value *second = car(cdr(args));

    if (((*first).type != DOUBLE_TYPE && (*first).type != INT_TYPE) || \
        ((*second).type != DOUBLE_TYPE && (*second).type != INT_TYPE))
        evaluationError(4);

    if ((*first).type == INT_TYPE)
        (*result).d += (*first).i;
    else
        (*result).d += (*first).d;
    if ((*second).type == INT_TYPE)
        (*result).d -= (*second).i;
    else
        (*result).d -= (*second).d;

    return result;
}

Value *primDiv(Value *args) {

    if (!validArgs(args))
        evaluationError(4);

    Value *result = makeNull();
    (*result).type = DOUBLE_TYPE;
    (*result).d = 0;

    Value *first = car(args);
    Value *second = car(cdr(args));

    if (((*first).type != DOUBLE_TYPE && (*first).type != INT_TYPE) || \
        ((*second).type != DOUBLE_TYPE && (*second).type != INT_TYPE))
        evaluationError(4);

    if((*first).type == INT_TYPE && (*second).type == INT_TYPE)
        (*result).d = (double) (*first).i / (double) (*second).i;
    else if((*first).type == DOUBLE_TYPE && (*second).type == DOUBLE_TYPE)
        (*result).d = (double) (*first).d / (double) (*second).d;
    else if((*first).type == INT_TYPE && (*second).type == DOUBLE_TYPE)
        (*result).d = (double) (*first).i / (double) (*second).d;
    else
        (*result).d = (double) (*first).d / (double) (*second).i;

    return result;
}

Value *primLess(Value *args) {

    if (!validArgs(args))
        evaluationError(4);

    Value *first = car(args);
    Value *second = car(cdr(args));

    if (((*first).type != DOUBLE_TYPE && (*first).type != INT_TYPE) || \
        ((*second).type != DOUBLE_TYPE && (*second).type != INT_TYPE))
        evaluationError(4);

    Value *trueResult = makeNull();
    Value *falseResult = makeNull();
    (*trueResult).type = BOOL_TYPE;
    (*falseResult).type = BOOL_TYPE;
    (*trueResult).i = 1;
    (*falseResult).i = 0;

    if((*first).type == INT_TYPE && (*second).type == INT_TYPE) {
        if( (*first).i < (*second).i)
            return trueResult;
        else
            return falseResult;
    }
    else if((*first).type == DOUBLE_TYPE && (*second).type == DOUBLE_TYPE) {
        if ( (*first).d < (*second).d)
            return trueResult;
        else
            return falseResult;
    }
    else if((*first).type == INT_TYPE && (*second).type == DOUBLE_TYPE) {
        if ( (*first).i < (*second).d)
            return trueResult;
        else
            return falseResult;
    }
    else {
        if ( (*first).d < (*second).i)
            return trueResult;
        else
            return falseResult;
    }
    evaluationError(4);
    return makeNull();
}

Value *primGreater(Value *args) {

    if (!validArgs(args))
        evaluationError(4);

    Value *first = car(args);
    Value *second = car(cdr(args));

    if (((*first).type != DOUBLE_TYPE && (*first).type != INT_TYPE) || \
        ((*second).type != DOUBLE_TYPE && (*second).type != INT_TYPE))
        evaluationError(4);

    Value *trueResult = makeNull();
    Value *falseResult = makeNull();
    (*trueResult).type = BOOL_TYPE;
    (*falseResult).type = BOOL_TYPE;
    (*trueResult).i = 1;
    (*falseResult).i = 0;

    if((*first).type == INT_TYPE && (*second).type == INT_TYPE) {
        if( (*first).i > (*second).i)
            return trueResult;
        else
            return falseResult;
    }
    else if((*first).type == DOUBLE_TYPE && (*second).type == DOUBLE_TYPE) {
        if ( (*first).d > (*second).d)
            return trueResult;
        else
            return falseResult;
    }
    else if((*first).type == INT_TYPE && (*second).type == DOUBLE_TYPE) {
        if ( (*first).i > (*second).d)
            return trueResult;
        else
            return falseResult;
    }
    else {
        if ( (*first).d > (*second).i)
            return trueResult;
        else
            return falseResult;
    }
    evaluationError(4);
    return makeNull();
}

Value *primEqual(Value *args) {

    if (!validArgs(args))
        evaluationError(4);

    Value *first = car(args);
    Value *second = car(cdr(args));

    if (((*first).type != DOUBLE_TYPE && (*first).type != INT_TYPE) || \
        ((*second).type != DOUBLE_TYPE && (*second).type != INT_TYPE))
        evaluationError(4);

    Value *trueResult = makeNull();
    Value *falseResult = makeNull();
    (*trueResult).type = BOOL_TYPE;
    (*falseResult).type = BOOL_TYPE;
    (*trueResult).i = 1;
    (*falseResult).i = 0;

    if((*first).type == INT_TYPE && (*second).type == INT_TYPE) {
        if( (*first).i == (*second).i)
            return trueResult;
        else
            return falseResult;
    }
    else if((*first).type == DOUBLE_TYPE && (*second).type == DOUBLE_TYPE) {
        if ( (*first).d == (*second).d)
            return trueResult;
        else
            return falseResult;
    }
    else if((*first).type == INT_TYPE && (*second).type == DOUBLE_TYPE) {
        if ( (*first).i == (*second).d)
            return trueResult;
        else
            return falseResult;
    }
    else {
        if ( (*first).d == (*second).i)
            return trueResult;
        else
            return falseResult;
    }
    evaluationError(4);
    return makeNull();
}

// Deal with cons primitive
Value *primCons(Value *args) {
    if (!validArgs(args))
        evaluationError(13);
    Value *first = car(args);
    Value *second = car(cdr(args));
    Value *result = cons(first, second);
    return result;
}

// interpret function that takes in a parsed tree as input and evaluates
// each s-expression in that parse tree
void interpret(Value *tree){
    Frame *globalFrame = talloc(sizeof(Frame));
    (*globalFrame).bindings = makeNull();
    (*globalFrame).parent = NULL;
    // add "+", "car", "cdr", "cons" and "null?" here
    bind("car", primCar, globalFrame);
    bind("cdr", primCdr, globalFrame);
    bind("null?", primNull, globalFrame);
    bind("+", primAdd, globalFrame);
    bind("-", primSub, globalFrame);
    bind("*", primMult, globalFrame);
    bind("<", primLess, globalFrame);
    bind(">", primGreater, globalFrame);
    bind("=", primEqual, globalFrame);
    bind("/", primDiv, globalFrame);
    bind("modulo", primMod, globalFrame);
    bind("cons", primCons, globalFrame);

    if ((*car(tree)).type == NULL_TYPE)
        evaluationError(5);

    while((*tree).type != NULL_TYPE) {
        displayResult(eval(car(tree), globalFrame));
        tree = cdr(tree);
    }
}

// Evaluate an if scheme expression
Value *evalIf(Value *args, Frame *frame) {
    Value *trueResult;
    Value *falseResult;
    Value *condEval = eval(car(args), frame);
    if ((*condEval).type == BOOL_TYPE) {
        if ((*condEval).i == 0) {
            if ((*cdr(cdr(args))).type != NULL_TYPE){
                falseResult = eval(car(cdr(cdr(args))), frame);
                return falseResult;
            }
            else 
                return makeNull();
        } 
        else {
            trueResult = eval(car(cdr(args)), frame);
            return trueResult;
        }
    }
    else {
        trueResult = eval(car(cdr(args)), frame);
        return trueResult;
    }

    evaluationError(1);
    // To suppress warning.
    return makeNull();
}

// Check if we have a valid if statement
int validIf(Value *expr) {
    int elements = 0;
    while((*expr).type != NULL_TYPE) {
        elements++;
        expr = cdr(expr);
    }
    // Valid if statement if 3 or 4 elements
    if (elements == 3 || elements == 4)
        return 1;
    return 0;
}

// Go through each frame and search for most recent binding of
// symbol if it exists
Value *lookUpSymbol(Value *sym, Frame *frame){
    Frame *curFrame = frame;
    Value *curBinding = (*frame).bindings;
    // assert((*curFrame).parent);

    // While cur frame is not a null pointer
    while(curFrame) {
        curBinding = (*curFrame).bindings;
        // while current thing in bindings is not a nullType
        while((*curBinding).type != NULL_TYPE) {
            // We found the symbol in a binding list
            // Return the value associated with this symbol
            if (strcmp((*car(curBinding)).s, (*sym).s) == 0) {
                
                return car(cdr(curBinding));
            } 
            // Check the next item in the list of bindings 
            // for this frame
            else
                curBinding = cdr(cdr(curBinding));
        }
        // Move up a frame, because we didn't find it in this one
        curFrame = (*curFrame).parent;
    }
    Value *noBinding = makeNull();
    (*noBinding).type = VOID_TYPE;

    // We couldn't find a binding :(
    return noBinding;
}

// Evaluate a let expression
Value *evalLet(Value *args, Frame *frame) {
    if ((*args).type != CONS_TYPE)
        evaluationError(2); 

    Value *body = cdr(args);

    if ((*body).type != CONS_TYPE)
        evaluationError(2); // no body!

    Frame *babyFrame = talloc(sizeof(Frame));
    (*babyFrame).parent = frame;

    Value *listOfBindings = makeNull();
    Value* bindings = car(args);

    while ((*bindings).type != NULL_TYPE) {
        // Check validity of binding syntax
        if ((*bindings).type != CONS_TYPE)
            evaluationError(2);

        Value *varHolder = car(bindings);
        // check for ANY bindings (if none, skip this block)
        if ((*varHolder).type != NULL_TYPE){
            // check that this binding is contained in a list
            if ((*varHolder).type != CONS_TYPE)
                evaluationError(2);

            Value *valHolder = cdr(varHolder);

            if ((*valHolder).type != CONS_TYPE)
                evaluationError(2);
            
            if ((*cdr(valHolder)).type != NULL_TYPE)
                evaluationError(2);
            

            Value *var = car(varHolder);
            Value *val = car(valHolder);
            if ((*var).type != SYMBOL_TYPE)
                evaluationError(2);

            // reversed order (for now) to preserve order from 'let'
            listOfBindings = cons(var,listOfBindings);
            listOfBindings = cons(val,listOfBindings);
        }
        bindings = cdr(bindings);
    }

    listOfBindings = reverse(listOfBindings);
    (*babyFrame).bindings = listOfBindings;

    while((*cdr(body)).type != NULL_TYPE){
        eval(car(body),babyFrame);
        body = cdr(body);
    }

    return eval(car(body),babyFrame);
}

// Evaluate a let* expression
Value *evalLetStar(Value *args, Frame *frame) {
    if ((*args).type != CONS_TYPE)
        evaluationError(19); 

    Value *body = cdr(args);

    if ((*body).type != CONS_TYPE)
        evaluationError(19); // no body!


    Frame *curFrame = frame;

    Value* bindings = car(args);

    while ((*bindings).type != NULL_TYPE) {

        // new baby frame
        Frame *babyFrame = talloc(sizeof(Frame));
        (*babyFrame).parent = curFrame;
        (*babyFrame).bindings = makeNull();

        //  Check validity of binding syntax
        if ((*bindings).type != CONS_TYPE)
            evaluationError(19);

        Value *varHolder = car(bindings);
        // check for ANY bindings (if none, skip this block)
        if ((*varHolder).type != NULL_TYPE){
            // check that this binding is contained in a list
            if ((*varHolder).type != CONS_TYPE)
                evaluationError(19);

            Value *valHolder = cdr(varHolder);

            if ((*valHolder).type != CONS_TYPE)
                evaluationError(19);
            
            if ((*cdr(valHolder)).type != NULL_TYPE)
                evaluationError(19);
            

            Value *var = car(varHolder);
            Value *val = car(valHolder);
            if ((*var).type != SYMBOL_TYPE)
                evaluationError(19);

            (*babyFrame).bindings = cons(eval(val, curFrame), (*babyFrame).bindings);
            (*babyFrame).bindings = cons(var, (*babyFrame).bindings);
        }
        bindings = cdr(bindings);
        curFrame = babyFrame;

    }

    while((*cdr(body)).type != NULL_TYPE){
        eval(car(body), curFrame);
        body = cdr(body);
    }

    return eval(car(body), curFrame);
}

// Evaluate a letrec expression
Value *evalLetRec(Value *args, Frame *frame) {
    if ((*args).type != CONS_TYPE)
        evaluationError(20); 

    Value *body = cdr(args);

    if ((*body).type != CONS_TYPE)
        evaluationError(20); // no body!

    Frame *babyFrame = talloc(sizeof(Frame));
    (*babyFrame).parent = frame;

    Value *listOfBindings = makeNull();
    Value* bindings = car(args);

    while ((*bindings).type != NULL_TYPE) {
        // Check validity of binding syntax
        if ((*bindings).type != CONS_TYPE)
            evaluationError(20);

        Value *varHolder = car(bindings);
        // check for ANY bindings (if none, skip this block)
        if ((*varHolder).type != NULL_TYPE){
            // check that this binding is contained in a list
            if ((*varHolder).type != CONS_TYPE)
                evaluationError(20);

            Value *valHolder = cdr(varHolder);

            if ((*valHolder).type != CONS_TYPE)
                evaluationError(20);
            
            if ((*cdr(valHolder)).type != NULL_TYPE)
                evaluationError(20);
            

            Value *var = car(varHolder);
            Value *val = car(valHolder);
            if ((*var).type != SYMBOL_TYPE)
                evaluationError(20);

            // reversed order (for now) to preserve order from 'let'
            listOfBindings = cons(var,listOfBindings);
            listOfBindings = cons(val,listOfBindings);
        }
        bindings = cdr(bindings);
    }

    listOfBindings = reverse(listOfBindings);
    (*babyFrame).bindings = listOfBindings;

    Frame *emptyChild = talloc(sizeof(Frame));
    (*emptyChild).parent = babyFrame;
    (*emptyChild).bindings = makeNull();

    while((*cdr(body)).type != NULL_TYPE){
        eval(car(body),emptyChild);
        body = cdr(body);
    }

    return eval(car(body),emptyChild);
}

Value *evalEach(Value *args, Frame *frame){
    Value *cur = args;
    Value *evaledArgs = makeNull();
    while((*cur).type != NULL_TYPE) {
        evaledArgs = cons(eval(car(cur), frame), evaledArgs);
        //displayResult(eval((car(cur)), frame));
        cur = cdr(cur);
    }
    evaledArgs = reverse(evaledArgs);
    return evaledArgs;
}

Value *evalSet(Value *args, Frame *frame) {

    if(!validArgs(args))
        evaluationError(4);

    if((*lookUpSymbol(car(args), frame)).type == VOID_TYPE)
        evaluationError(4);

    Value *toReturn = makeNull();
    (*toReturn).type = VOID_TYPE;

    Value *var = car(args);
    Value *toSet = eval(car(cdr(args)), frame);
    Frame *curFrame = frame;
    Value *curBinding = (*frame).bindings;
    // assert((*curFrame).parent);

    // While cur frame is not a null pointer
    while(curFrame) {
        curBinding = (*curFrame).bindings;
        while((*curBinding).type != NULL_TYPE) {
            if (strcmp((*car(curBinding)).s, (*var).s) == 0) {
                (*cdr(curBinding)).c.car = toSet;
                return toReturn;
            } 
            else
                curBinding = cdr(cdr(curBinding));
        }
        curFrame = (*curFrame).parent;
    }
    evaluationError(4);
    return makeNull();
}

Value *evalBegin(Value *args, Frame *frame) {
    Value *toReturn;
    Value *evaledArgs;

    if ((*args).type == NULL_TYPE){
        (*toReturn).type = VOID_TYPE;
        return toReturn;
    }
    
    evaledArgs = evalEach(args, frame);

    Value *curArg = evaledArgs;

    while((*cdr(curArg)).type != NULL_TYPE)
        curArg = cdr(curArg);

    toReturn = car(curArg);
    return toReturn;
}

Value *evalPrim(Value *prim, Value *args, Frame *frame) {
    Value *evaledArgs = evalEach(args, frame);
    // write all our primitive functions to take args
    return ((*prim).pf)(evaledArgs);
}

Value *evalAnd(Value *args, Frame *frame){
    // check for proper syntax:
    if ((*args).type != CONS_TYPE || \
        (*cdr(args)).type != CONS_TYPE || \
        (*cdr(cdr(args))).type != NULL_TYPE) {
        evaluationError(16);
    }

    Value *result = makeNull();
    (*result).type = BOOL_TYPE;
    (*result).i = 1;

    Value *curBool = eval(car(args), frame);
    if ((*curBool).type != BOOL_TYPE) {
        evaluationError(16);
    }
    if ((*curBool).i == 0) {
        (*result).i = 0;
        return result;
    }

    curBool = eval(car(cdr(args)), frame);
    if ((*curBool).type != BOOL_TYPE) {
        evaluationError(16);
    }
    if ((*curBool).i == 0) {
        (*result).i = 0;
        return result;
    }
    return result;
}

Value *evalOr(Value *args, Frame *frame){
    // check for proper syntax:
    if ((*args).type != CONS_TYPE || \
        (*cdr(args)).type != CONS_TYPE || \
        (*cdr(cdr(args))).type != NULL_TYPE) {
        evaluationError(17);
    }

    Value *result = makeNull();
    (*result).type = BOOL_TYPE;
    (*result).i = 0;

    Value *curBool = eval(car(args), frame);
    if ((*curBool).type != BOOL_TYPE) {
        evaluationError(17);
    }
    if ((*curBool).i == 1) {
        (*result).i = 1;
        return result;
    }

    curBool = eval(car(cdr(args)), frame);
    if ((*curBool).type != BOOL_TYPE) {
        evaluationError(17);
    }
    if ((*curBool).i == 1) {
        (*result).i = 1;
        return result;
    }
    return result;
}

Value *evalCond(Value *args, Frame *frame) {
    Value *toReturn = makeNull();
    (*toReturn).type = VOID_TYPE;
    Value *curArg = args;

    while((*curArg).type != NULL_TYPE) {
        if ((*curArg).type != CONS_TYPE)
            evaluationError(4);

        if ((*car(car(curArg))).type == CONS_TYPE) {
            Value *evaledCond = eval(car(car(curArg)), frame);
            if ((*evaledCond).type == BOOL_TYPE) {
                if ((*evaledCond).i == 1)
                    return eval(car(cdr(car(curArg))), frame); 
            }
            else 
                evaluationError(4);
        }
        else if ((*car(car(curArg))).type == SYMBOL_TYPE) {
            if (strcmp((*car(car(curArg))).s, "else") == 0)
                return eval(car(curArg), frame);
            else
                evaluationError(4);
        }
        else
            evaluationError(4);
        curArg = cdr(curArg);
    }
    return toReturn;
}

Value *evalElse(Value *args, Frame *frame) {
    if ((*args).type != CONS_TYPE && (*cdr(args)).type != NULL_TYPE)
        evaluationError(4);
    return eval(car(args), frame);
}

Value *evalLambda(Value *args, Frame *frame){

    // Args currently contains params and function body
    // Make sure it's a list
    if ((*args).type != CONS_TYPE)
        evaluationError(9);

    // Get list of params out of args, make sure it's a list
    // (can be an empty list)
    Value *params = car(args);
    if ((*params).type != CONS_TYPE)
        evaluationError(7);

    // Get function body, make sure it's a list
    Value *functionBody = cdr(args);
    if ((*params).type != CONS_TYPE)
        evaluationError(8);

    Value *result = makeNull();
    (*result).type = CLOSURE_TYPE;
    (*result).cl.frame = frame;

    Value *cur = params;
    // check that the formal params are all SYMBOL_TYPE
    while ((*cur).type != NULL_TYPE){
        if((*car(cur)).type != SYMBOL_TYPE && (*car(cur)).type != NULL_TYPE)
            evaluationError(7);
        cur = cdr(cur);
    }

    (*result).cl.paramNames = params;
    if ((*car(params)).type == NULL_TYPE)
        (*result).cl.paramNames = makeNull();

    if ((*cdr(args)).type == NULL_TYPE)
        evaluationError(8);

    (*result).cl.functionCode = functionBody;
    return result;
}

Value *apply(Value *operator, Value *args) {
    flag("entered apply\n");

    Frame *babyFrame = talloc(sizeof(Frame));
    (*babyFrame).parent = (*operator).cl.frame;
    Value *bindings = makeNull();

    Value *curParam = (*operator).cl.paramNames;
    Value *curArg = args;

    if (length(curParam) != length(curArg)) {
        evaluationError(10);
    }

    while ((*curParam).type != NULL_TYPE && (*curArg).type != NULL_TYPE) {
        // Push the formal parameter (the symbol)
        bindings = cons(car(curParam),bindings);
        // Push the value associated with this parameter
        bindings = cons(car(curArg), bindings);
        curParam = cdr(curParam);
        curArg = cdr(curArg);
    }

    // to preserve order
    bindings = reverse(bindings);
    (*babyFrame).bindings = bindings;

    Value *curEval = (*operator).cl.functionCode;

    while((*cdr(curEval)).type != NULL_TYPE){
        eval(car(curEval), babyFrame);
        curEval = cdr(curEval);
    }
    return eval(car(curEval), babyFrame);
    // return eval(car((*operator).cl.functionCode), babyFrame);
}

Value *addDefinition(Value *args, Frame *frame) {
    Value *result = makeNull();
    (*result).type = VOID_TYPE;

    // check syntax
    if ((*args).type != CONS_TYPE || (*car(args)).type != SYMBOL_TYPE \
        || (*cdr(args)).type != CONS_TYPE || (*cdr(cdr(args))).type != NULL_TYPE){
        evaluationError(6);
    }

    Value *var = car(args);
    Value *val = car(cdr(args));

    // (most of) these cases should never happen; checking just in case
    if ((*val).type == NULL_TYPE || (*val).type == PTR_TYPE || \
        (*val).type == OPEN_TYPE || (*val).type == CLOSE_TYPE || \
        (*val).type == VOID_TYPE){
        evaluationError(6);
    }
    // get to global frame
    while ((*frame).parent != NULL) {
        frame = (*frame).parent;
    }
    // check to see if function is already in global frame
    // if so, modify the binding
    Value *curBinding = (*frame).bindings;
    int foundOld = 0;
    while ((*curBinding).type != NULL_TYPE) {
        if (strcmp((*car(curBinding)).s, (*var).s) == 0) {
            (*cdr(curBinding)).c.car = val;
            foundOld = 1;
            break;
        }
        curBinding = cdr(cdr(curBinding));
    }

    // add new binding, use 'reverse' to preserve logical order
    // (oldest bindings should come at the end of the list)
    if (foundOld == 0) {
        Value *bindings = reverse((*frame).bindings);
        bindings = cons(var, bindings);
        bindings = cons(val, bindings);
        bindings = reverse(bindings);
        (*frame).bindings = bindings;
    }
    return result;
}

// evaluate scheme expression
Value *eval(Value *expr, Frame *frame) {
    flag("STARTED Eval\n");
    Value *first;
    Value *args;
    Value *result;
    result = makeNull();

    // Switch to deal with each case appropriately
    switch ((*expr).type)  {
        case INT_TYPE:
            flag("INT_TYPE\n");
            return expr;

        case SYMBOL_TYPE: 
            flag("SYMBOL_TYPE\n");
            result = lookUpSymbol(expr, frame);
            if ((*result).type == VOID_TYPE)
                evaluationErrorUndefVar((*expr).s);
            else
                return eval(result, frame);

        case STR_TYPE:
            flag("STR_TYPE\n");
            return expr;
            break;

        case NULL_TYPE:
            flag("NULL_TYPE\n");
            result = makeNull();
            return result;

        case PTR_TYPE:
            flag("PTR_TYPE\n");
            evaluationError(4);
            break;

        case OPEN_TYPE:
            flag("OPEN_TYPE\n");
            evaluationError(4);
            break;

        case CLOSE_TYPE:
            flag("CLOSE_TYPE\n");
            evaluationError(4);
            break;

        case DOUBLE_TYPE:
            flag("DOUBLE_TYPE\n");
            return expr;

        case BOOL_TYPE:
            flag("BOOL_TYPE\n");
            return expr;
           
        case CONS_TYPE: 
            flag("CONS_TYPE\n");
            first = car(expr);
            args = cdr(expr);

            // printf("TYPE OF FIRST: %d\n", (*first).type);

            if ((*first).type == SYMBOL_TYPE) {
                // Special case: if
                if (strcmp((*first).s,"if") == 0) {
                    if (validIf(expr)) {
                        return evalIf(args,frame);
                    }
                    else {
                        evaluationError(1);
                    }
                }

                // Special case: let
                else if (strcmp((*first).s,"let") == 0) {
                    flag("Detected 'let' \n");
                    return evalLet(args, frame);
                }

                // Special case: let*
                else if (strcmp((*first).s,"let*") == 0) {
                    flag("Detected 'let*' \n");
                    return evalLetStar(args, frame);
                }

                //Special case: letrec
                else if (strcmp((*first).s,"letrec") == 0) {
                    flag("Detected 'letrec' \n");
                    return evalLetRec(args, frame);
                }

                // Special case: quote
                else if (strcmp((*first).s,"quote") == 0) {
                    flag("Detected 'quote' \n");
                    return car(args);
                }

                // Special case: define
                else if (strcmp((*first).s,"define") == 0) {
                    flag("Detected 'define' \n");
                    return addDefinition(args,frame);
                }

                // Special case: lambda
                else if (strcmp((*first).s,"lambda") == 0) {
                    flag("Detected 'lambda' \n");
                    return evalLambda(args,frame);
                }

                // Special case: and
                else if (strcmp((*first).s,"and") == 0) {
                    flag("Detected 'and' \n");
                    return evalAnd(args,frame);
                }

                // Special case: or
                else if (strcmp((*first).s,"or") == 0) {
                    flag("Detected 'or' \n");
                    return evalOr(args,frame);
                }

                else if (strcmp((*first).s, "begin") == 0) {
                    flag("Detected 'begin' \n");
                    return evalBegin(args, frame);
                }

                else if (strcmp((*first).s, "set!") == 0) {
                    flag("Detected 'set!' \n");
                    return evalSet(args, frame);
                }

                else if (strcmp((*first).s, "cond") == 0) {
                    flag("Detected 'cond' \n");
                    return evalCond(args, frame);
                }

                else if (strcmp((*first).s, "else") == 0) {
                    flag("Detected 'cond' \n");
                    return evalElse(args, frame);
                }

                flag("not a special form\n");
                // If not a special form, evaluate the first, evaluate the args, then
                // apply the first to the args.
                Value *evaledOperator = eval(first, frame);

                if ((*evaledOperator).type == PRIMITIVE_TYPE) {
                    return evalPrim(evaledOperator, args, frame);
                } else if ((*evaledOperator).type == CLOSURE_TYPE) {
                    Value *evaledArgs = evalEach(args, frame);
                    return apply(evaledOperator,evaledArgs);
                } else {
                    evaluationError(18);
                }
            }
            else if ((*first).type == NULL_TYPE) {
                return eval(first,frame);
            }
            else if ((*first).type == CONS_TYPE) {
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                return apply(evaledOperator,evaledArgs);
            }
            else {
                // printf("about to display expr: \n");
                // display(expr);
                //printf("type: %d \n",(*first).type);
                //display(expr);
                //evaluationError(11);
                return expr;
            }
        case VOID_TYPE:
            printf("Why are you trying to eval a VOID_TYPE?!");
            // return expr;
            evaluationError(4);
            break;
        case CLOSURE_TYPE:
            return expr;
        case PRIMITIVE_TYPE:
            return expr;
                
    }
    return result;  
}

