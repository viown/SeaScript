#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lex.h"
#include "./seavm/vm.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

void test(lex_Object* obj) {
	char n[10] = "hi";
	strcpy(obj->tokens[0].value, n);
}

void visualize_token(Token* token) {
	printf("<SPECIFIER='%d', VALUE='%s'>\n", token->token,token->value);
}

int main() {
	char* source = "var a = 533\nvar b = 200 ";
	lex_Object object;
	lexObject_init(&object, source);
	lex(&object);
	for (int i = 0; i < object.token_used; ++i) {
		visualize_token(&object.tokens[i]);
	}
	lex_free(&object);
	return 0;
}

