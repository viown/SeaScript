#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lex.h"
#include "./parser.h"
#include "./seavm/bytecode.h"
#include "./seavm/vm.h"

#include "./debug.h"

int main() {
	char* source = "global a = 15; global b = 20;";
	// lex test
	lex_Object object;
	lexObject_init(&object, source);
	lex(&object);
	visualize_tokens(&object);
	State* s = parse(object);
	free(s); /* todo: also free internals */
	lex_free(&object);
	free(source);
	return 0;
}

