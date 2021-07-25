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
    InstructionHolder instructionHolder = read_from_file(path);
    Instruction* instructions = instructionHolder.instructions;

    for (int i = 0; i < instructionHolder.length; ++i) {
        if (instructions[i].op == LBL) {
            printf("\n");
        }
        const char* instruction = instruction_to_string(instructions[i].op);
        printf("%s\t", instruction);
        for (int j = 0; j < MAX_ARGS; ++j) {
            if (get_reader(instructions[i].op)->bytes_to_read == 0)
                break;
            printf("%lld ", instructions[i].args[j]);
        }
        if (instructions[i].op == CALLC) {
            int func_id = (int)instructions[i].args[0];
            ss_BaseFunction f = ss_functions[func_id];
            printf("\t; function: %s", f.name);
        }
        printf("\n");
    }
    free_holder(&instructionHolder);
    return 0;
}

int execute_bytecode(char* path) {
    VirtualMachine virtual_machine;
    vm_init(&virtual_machine, ss_functions);
    InstructionHolder holder = read_from_file(path);
    int exec = vm_execute(&virtual_machine, holder.instructions, holder.length);
    free_holder(&holder);
    vm_free(&virtual_machine);
    return exec;
}

int compile_and_run(CommandLineFlags flags, char* path) {
    VirtualMachine virtual_machine;
    vm_init(&virtual_machine, ss_functions);
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
    ReferenceTable reftable = compile(&s);
    if (flags.preserve_bytecode) {
        char* bytecode_path = path;
        bytecode_path[strlen(bytecode_path)-1] = 'b';
        save_to_file(reftable.map->instructions, reftable.map->length, bytecode_path);
    }
    free_ParseObject(&s);
    lex_free(&object);
    free_and_null(source_code);
    if (!flags.no_run) {
        int ret = vm_execute(&virtual_machine, reftable.map->instructions, reftable.map->length);
        reftable_free(&reftable);
        vm_free(&virtual_machine);
        return ret;
    } else {
        reftable_free(&reftable);
        vm_free(&virtual_machine);
        return 0;
    }
}

void read_flags(CommandLineFlags* flags, int argc, char** argv) {
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--view") == 0)
            flags->is_view = true;
        else if (!strcmp(argv[i], "--preserve"))
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
            ICONST, {250.50}
        },
        {
            STORE, {0}
        },
        {
            ICONST, {100}
        },
        {
            STORE, {0}
        },
        {
            ICONST, {25}
        },
        {
            LOAD, {0}
        },
        {
            IPRINT, {}
        },
        {
            EXIT, {0}
        },
    };

    VirtualMachine vm;

    vm_init(&vm, ss_functions);

    //save_to_file(instructions, LEN(instructions), "lol.ssb");

	int ret = vm_execute(&vm, instructions, LEN(instructions));
	vm_free(&vm);
	return ret;
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
