#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lex.h"
#include "./parser.h"
#include "./seavm/bytecode.h"
#include "./seavm/vm.h"
#include "./debug.h"
#include "./ssfunctions.h"
#include "./compiler.h"

#define VERSION "1.0.0"

typedef struct {
    bool is_view;
    bool preserve_bytecode;
    bool visualize_tokens;
    bool parser_print;
} CommandLineFlags;

CommandLineFlags init_flags() {
    CommandLineFlags flags;
    flags.is_view = false;
    flags.preserve_bytecode = false;
    flags.visualize_tokens = false;
    flags.parser_print = false;
    return flags;
}

void get_extension(char* file_name, char* modify) {
    char extension[255];
    int len = 0;
    char* extension_start = NULL;
    for (int i = 0; i < strlen(file_name); ++i) {
        if (file_name[i] == '.') {
            extension_start = &file_name[i];
            break;
        }
    }
    if (extension_start == NULL) {
        ss_throw("Bad Argument: Valid filename must be passed");
    } else {
        while (*extension_start != '\0') {
            extension[len++] = *extension_start;
            extension_start++;
        }
    }
    strcpy(modify, extension);
}

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

bool test_language() {
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

bool test_vm() {
    Instruction instructions[] = {
        {
            ICONST, {50}
        },
        {
            ICONST, {50}
        },
        {
            ADD, {}
        },
        {
            IPRINT, {}
        },
        {
            ICONST, {25}
        },
        {
            CALLC, {0}
        }
    };

    Vm vm;

    vm_init(&vm, 100, ss_functions);
    return vm_execute(&vm, instructions, LEN(instructions));
}

int main(int argc, char** argv) {
    CommandLineFlags flags = init_flags();
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--view") == 0)
            flags.is_view = true;
        else if (!strcmp(argv[i], "--preserve-bytecode"))
            flags.preserve_bytecode = true;
        else if (!strcmp(argv[i], "--visualize-tokens"))
            flags.visualize_tokens = true;
        else if (!strcmp(argv[i], "--parser-print"))
            flags.parser_print = true;
        else if (argv[i][0] == '-' && argv[i][1] == '-')
            ss_throw("Invalid flag '%s'", argv[i]);
    }
    if (argc >= 2) {
        if (strcmp(argv[1], "--version") == 0) {
            printf("V%s", VERSION);
            return 0;
        } else {
            char extension[255];
            get_extension(argv[1], extension);
            if (strcmp(extension, ".ssb") == 0) {
                if (!flags.is_view) {
                    /* TODO: Execute bytecode file */
                } else {
                    /* TODO: Visualize bytecode to instructions*/
                    Bytecode bytecode;
                    read_from_file(&bytecode, argv[1]);
                    Instruction* instructions = to_instructions(&bytecode);
                    for (int i = 0; i < bytecode.length; ++i) {
                        const char* instruction = instruction_to_string(instructions[i].op);
                        printf("%s ", instruction);
                        for (int j = 0; j < 3; ++j) {
                            if (instructions[i].args[j] == 0)
                                break;
                            printf("%d ", instructions[i].args[j]);
                        }
                        printf("\n");
                    }
                }
            } else if (!strcmp(extension, ".ssc")) {
                char* source_code = read_file(argv[1]);
                lex_Object object;
                lexObject_init(&object, source_code);
                lex(&object);
                if (flags.visualize_tokens) {
                    if (object.token_used > 2500) {
                        printf("Tokens too big to visualize");
                        return 1;
                    }
                    visualize_tokens(&object);
                    return 0;
                }
                ParseObject s = parse(object);
                if (flags.parser_print) {
                    visualize_states(&s);
                    return 0;
                }
                InstructionMap map = compile(&s);
                Bytecode bytecode;
                to_bytecode(&bytecode, map.instructions, map.length);
                if (flags.preserve_bytecode) {
                    save_to_file(&bytecode, "test.ssb");
                }
                free_ParseObject(&s);
                lex_free(&object);
            } else {
                ss_throw("Bad Argument: Invalid extension '%s'", extension);
            }
        }
    }
    return 0;
}

