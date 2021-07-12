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
    bool no_run;
} CommandLineFlags;

CommandLineFlags init_flags() {
    CommandLineFlags flags;
    flags.is_view = false;
    flags.preserve_bytecode = false;
    flags.visualize_tokens = false;
    flags.parser_print = false;
    flags.no_run = false;
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

int visualize_bytecode(char* path) {
    Bytecode bytecode;
    read_from_file(&bytecode, path);
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
    free(instructions);
    return 0;
}

int execute_bytecode(char* path) {
    VirtualMachine virtual_machine;
    vm_init(&virtual_machine, 500, ss_functions);
    Bytecode bytecode;
    read_from_file(&bytecode, path);
    Instruction* instructions = to_instructions(&bytecode);
    int exec = vm_execute(&virtual_machine, instructions, bytecode.length);
    free_bytecode(&bytecode);
    free(instructions);
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
            ss_throw("Tokens too big to visualize");
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
    if (flags.preserve_bytecode) {
        Bytecode bytecode;
        to_bytecode(&bytecode, map.instructions, map.length);
        char* bytecode_path = path;
        bytecode_path[strlen(bytecode_path)-1] = 'b'; /* change extension from .ssc to .ssb */
        save_to_file(&bytecode, bytecode_path);
        free_bytecode(&bytecode);
    }
    free_ParseObject(&s);
    lex_free(&object);
    if (!flags.no_run) {
        return vm_execute(&virtual_machine, map.instructions, map.length);
    } else {
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
        else if (argv[i][0] == '-' && argv[i][1] == '-')
            ss_throw("Invalid flag '%s'", argv[i]);
    }
}

int vm_test() {
    Instruction instructions[] = {
        {
            CALL, {0}
        },
        {
            IPRINT, {0}
        },
        {
            EXIT, {0}
        },
        {
            LBL, {0}
        },
        {
            ICONST, {3001}
        },
        {
            RET, {}
        },
    };
	VirtualMachine vm;

	vm_init(&vm, 100, ss_functions);

	//Bytecode bytecode;
	//to_bytecode(&bytecode, instructions, LEN(instructions));
	//save_to_file(&bytecode, "test.ssb");

	return vm_execute(&vm, instructions, LEN(instructions));
}
int main(int argc, char** argv) {
    return vm_test();

    CommandLineFlags flags = init_flags();
    read_flags(&flags, argc, argv);
    if (argc >= 2) {
        if (strcmp(argv[1], "--version") == 0) {
            printf("V%s", VERSION);
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

