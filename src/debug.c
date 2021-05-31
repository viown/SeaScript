#include <stdio.h>
#include "./debug.h"

void visualize_tokens(lex_Object* object) {
	for (int i = 0; i < object->token_used; ++i) {
		printf("<SPECIFIER='%d', VALUE='%s'>\n", object->tokens[i].token, object->tokens[i].value);
	}
}

void visualize_states(ParseObject* object) {
	State* begin = object->begin;
	//ss_Variable v = *(ss_Variable*)begin->state;
	//ss_Identifier x = *(ss_Identifier*)v.states[0].state;
	//printf("%s", v.variable_name);
}
