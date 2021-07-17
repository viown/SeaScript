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
    bool is_view : 1;
    bool preserve_bytecode : 1;
    bool visualize_tokens : 1;
    bool parser_print : 1;
    bool no_run : 1;
    bool check_version : 1;
} CommandLineFlags;

CommandLineFlags init_flags() {
    CommandLineFlags flags;
    flags.is_view = false;
    flags.preserve_bytecode = false;
    flags.visualize_tokens = false;
    flags.parser_print = false;
    flags.no_run = false;
    flags.check_version = false;
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
    extension[len++] = '\0';
    strcpy(modify, extension);
}

char* read_file(const char* path) {
    FILE *fp = fopen(path, "r");
    long file_size = get_file_size(path);
    char* source = (char*)ss_malloc(file_size + 2);
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

int visualize_bytecode(char* path) {
    Bytecode bytecode = read_from_file(path);
    Instruction* instructions = to_instructions(&bytecode);

    for (int i = 0; i < bytecode.length; ++i) {
        if (instructions[i].op == LBL) {
            printf("\n");
        }
        const char* instruction = instruction_to_string(instructions[i].op);
        printf("%s ", instruction);
        for (int j = 0; j < 3; ++j) {
            if (instructions[i].args[j] == 0)
                break;
            printf("%d ", instructions[i].args[j]);
        }
        printf("\n");
    }
    free_and_null(instructions);
    free_bytecode(&bytecode);
    return 0;
}

int execute_bytecode(char* path) {
    VirtualMachine virtual_machine;
    vm_init(&virtual_machine, 500, ss_functions);
    Bytecode bytecode = read_from_file(path);
    Instruction* instructions = to_instructions(&bytecode);
    int exec = vm_execute(&virtual_machine, instructions, bytecode.length);
    free_bytecode(&bytecode);
    free_and_null(instructions);
    return exec;
}

int compile_and_run(CommandLineFlags flags, char* path) {
    VirtualMachine virtual_machine;
    vm_init(&virtual_machine, 500, ss_functions);
    char* source_code = read_file(path);
    if (source_code == NULL)
        ss_throw("File '%s' could not be found", path);
    lex_Object object;
    lexObject_init(&object, source_code);
    lex(&object);
    if (flags.visualize_tokens) {
        if (object.token_used > 2500) {
            lex_free(&object);
            vm_free(&virtual_machine);
            free_and_null(source_code);
            ss_throw("Tokens too big to visualize");
        }
        visualize_tokens(&object);
        lex_free(&object);
        vm_free(&virtual_machine);
        free_and_null(source_code);
        return 0;
    }
    ParseObject s = parse(object);
    if (flags.parser_print) {
        visualize_states(&s);
        free_ParseObject(&s);
        lex_free(&object);
        free_and_null(source_code);
        vm_free(&virtual_machine);
        return 0;
    }
    InstructionMap map = compile(&s);
    if (flags.preserve_bytecode) {
        Bytecode bytecode;
        to_bytecode(&bytecode, map.instructions, map.length);
        char* bytecode_path = path;
        bytecode_path[strlen(bytecode_path)-1] = 'b';
        save_to_file(&bytecode, bytecode_path);
        free_bytecode(&bytecode);
    }
    free_ParseObject(&s);
    lex_free(&object);
    free_and_null(source_code);
    if (!flags.no_run) {
        int ret = vm_execute(&virtual_machine, map.instructions, map.length);
        map_free(&map);
        vm_free(&virtual_machine);
        return ret;
    } else {
        map_free(&map);
        vm_free(&virtual_machine);
        return 0;
    }
}

void read_flags(CommandLineFlags* flags, int argc, char** argv) {
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--view") == 0)
            flags->is_view = true;
        else if (!strcmp(argv[i], "--preserve-bytecode"))
            flags->preserve_bytecode = true;
        else if (!strcmp(argv[i], "--visualize-tokens"))
            flags->visualize_tokens = true;
        else if (!strcmp(argv[i], "--parser-print"))
            flags->parser_print = true;
        else if (!strcmp(argv[i], "--norun"))
            flags->no_run = true;
        else if (!strcmp(argv[i], "--version"))
            flags->check_version = true;
        else if (argv[i][0] == '-' && argv[i][1] == '-')
            ss_throw("Invalid flag '%s'", argv[i]);
    }
}

int vm_test() {
    Instruction instructions[] = {
        {
            ICONST, {5}
        },
        {
            ICONST, {3}
        },
        {
            IADD, {}
        },
        {
            IPRINT, {}
        },
        {
            EXIT, {0}
        },
    };
	VirtualMachine vm;

	vm_init(&vm, 100, ss_functions);

	return vm_execute(&vm, instructions, LEN(instructions));
}
int main(int argc, char** argv) {
    CommandLineFlags flags = init_flags();
    read_flags(&flags, argc, argv);
    if (argc >= 2) {
        if (flags.check_version) {
            printf("V%s\n", VERSION);
            return 0;
        } else {
            char extension[255];
            get_extension(argv[1], extension);
            if (strcmp(extension, ".ssb") == 0) {
                if (!flags.is_view) {
                    return execute_bytecode(argv[1]);
                } else {
                    return visualize_bytecode(argv[1]);
                }
            } else if (!strcmp(extension, ".ssc")) {
                return compile_and_run(flags, argv[1]);
            } else {
                ss_throw("Bad Argument: Invalid extension '%s'", extension);
            }
        }
    }
    return 0;
}
