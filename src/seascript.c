#include <stdio.h>
#include "./lex.h"
#include "./seavm/vm.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))



int main() {
	printf("%d", is_punctuator('}'));
	return 0;
}



