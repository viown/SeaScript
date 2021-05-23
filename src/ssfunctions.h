#ifndef SS_FUNCTIONS_H
#define SS_FUNCTIONS_H
#include "./seavm/stack.h"

/*
	Functions that can be executed within SeaScript
	SeaVM executes these through the CALLC instruction
*/

void* print(Stack* stack);
void* exit(Stack* stack);

#endif // SS_FUNCTIONS_H
