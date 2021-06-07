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
	char* source = malloc(10000 + 2); /* FIXME: get_file_size() doesn't work well with linux */
	if (fp != NULL) {
		size_t len = fread(source, sizeof(char), 10000 , fp);
		source[len++] = ' '; /* closing whitespace */
		source[len++] = '\0';
		fclose(fp);
		return source;
	} else {
		return NULL;
	}
}

int main() {
	char* source = read_file("tests/parser_test.ssc");
	if (source != NULL ) {
		lex_Object object;
		lexObject_init(&object, source);
		lex(&object);
		visualize_tokens(&object);
		//ParseObject s = parse(object);
		//visualize_states(&s);
		//free_ParseObject(&s);
		lex_free(&object);
	}
	free(source);
	return 0;
}

