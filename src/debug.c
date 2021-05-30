#include <stdio.h>
#include "./debug.h"

void visualize_tokens(lex_Object* object) {
	for (int i = 0; i < object->token_used; ++i) {
		printf("<SPECIFIER='%d', VALUE='%s'>\n", object->tokens[i].token, object->tokens[i].value);
	}
}
