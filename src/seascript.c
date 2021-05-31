#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lex.h"
#include "./parser.h"
#include "./seavm/bytecode.h"
#include "./seavm/vm.h"

#include "./debug.h"

char* read_file(const char* path) {
	FILE *fp = fopen(path, "r");
	char* source = malloc(get_file_size(fp) + 1);
	if (fp != NULL) {
		size_t len = fread(source, sizeof(char), get_file_size(fp), fp);
		source[len++] = '\0';
		fclose(fp);
	}
	return source;
}

int main() {
	char* source = read_file("tests\\parser_test.ss");
	// lex test
	lex_Object object;
	lexObject_init(&object, source);
	lex(&object);
	//visualize_tokens(&object);
	ParseObject s = parse(object);
	visualize_states(&s);
	free(s.begin); /* todo: also free internals */
	lex_free(&object);
	free(source);
	return 0;
}

