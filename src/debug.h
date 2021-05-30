#ifndef SS_DEBUG_H
#include "./lex.h"
#include "./parser.h"

/*
	offers debugging functions
	not to be confused with a debug environment, seascript does not have one.
*/

void visualize_tokens(lex_Object* object);
void visualize_states(State* begin);

#endif // SS_DEBUG_H
