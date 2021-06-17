#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lex.h"
#include "./parser.h"
#include "./seavm/bytecode.h"
#include "./seavm/vm.h"
#include "./debug.h"
#include "./ssfunctions.h"

char* read_file(const char* path) {
    FILE *fp = fopen(path, "r");
    long file_size = get_file_size(path);
    char* source = (char*)malloc(file_size + 2);
    if (fp != NULL) {
        size_t len = fread(source, sizeof(char), file_size, fp);
        source[len++] = ' '; /* closing whitespace */
        source[len++] = '\0';
        fclose(fp);
        return source;
    } else {
        return NULL;
    }
}

int main() {
    char* source = read_file("tests/language/parser_test.ssc");
    if (source != NULL) {
        lex_Object object;
        test_call(true, lexObject_init(&object, source));
        test_call(true, lex(&object));
        test_call(false, visualize_tokens(&object));
        ParseObject s = parse(object);
        test_call(true, visualize_states(&s));
        test_call(true, free_ParseObject(&s));
        test_call(true, lex_free(&object));
    }
    free(source);
    return 0;
}

