#ifndef SS_DEBUG_H
#include "./lex.h"
#include "./parser.h"

#define ss_assert(condition) if (!(condition)) {ss_throw("%s:%d: %s: Assertion error '%s'", __FILE__, __LINE__, __func__, #condition);}

void ss_throw(const char* error, ...);
void visualize_tokens(lex_Object* object);
void visualize_states(ParseObject* object);
void visualize_token(Token* token);

#endif // SS_DEBUG_H
