#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lex.h"
#include "./parser.h"
#include "./seavm/bytecode.h"
#include "./seavm/vm.h"


void visualize_token(Token* token) {
	printf("<SPECIFIER='%d', VALUE='%s'>\n", token->token,token->value);
}

char* read_file(const char* path) {
	FILE* file = fopen(path, "r");
	char* buffer = malloc(get_file_size(file) + 1);
	int new_index = 0;
	int current_char = getc(file);
	while (current_char != EOF) {
		buffer[new_index++] = current_char;
		current_char = getc(file);
	}
	buffer[new_index++] = '\0';
	return buffer;
}

int main() {
	char* source = read_file("tests\\parser_test.ss");
	// lex test
	lex_Object object;
	lexObject_init(&object, source);
	lex(&object);
	for (int i = 0; i < object.token_used; ++i) {
		visualize_token(&object.tokens[i]);
	}
	State* s = parse(object);
	free(s); /* todo: also free internals */
	lex_free(&object);
	free(source);
	return 0;
}

