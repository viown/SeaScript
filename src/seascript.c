#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"
#include "shell.h"
#include "lex.h"
#include "parser.h"
#include "bytecode.h"
#include "vm.h"
#include "debug.h"
#include "ssfunctions.h"
#include "compiler.h"

bool shell_env = false;
bool debug_env = false;

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
        ss_throw("Bad Argument: Valid filename must be passed\n");
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
    StringPool pool = create_string_pool();
    InstructionHolder instructionHolder = read_from_file(path, &pool);
    Instruction* instructions = instructionHolder.instructions;

    // String Constants
    if (pool.length != 0) {
        printf("String Constants:\n");
        for (size_t i = 0; i < pool.length; ++i) {
            printf("%d - \"%s\"\n", i, pool.constants[i]);
        }
        printf("\n\n");
    }
    // Output instructions
    for (int i = 0; i < instructionHolder.length; ++i) {
        if (instructions[i].op == LBL) {
            printf("\n");
        }
        const char* instruction = instruction_to_string(instructions[i].op);
        printf("[%d] %s\t", i, instruction);
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
    free_string_pool(&pool);
    free_holder(&instructionHolder);
    return 0;
}

int execute_bytecode(char* path) {
    VirtualMachine virtual_machine;
    StringPool pool = create_string_pool();
    vm_init(&virtual_machine, ss_functions);
    InstructionHolder holder = read_from_file(path, &pool);
    int exec = vm_execute(&virtual_machine, &pool, holder.instructions, holder.length);
    free_holder(&holder);
    vm_free(&virtual_machine);
    free_string_pool(&pool);
    return exec;
}

int compile_and_run(CommandLineFlags flags, char* path) {
    VirtualMachine virtual_machine;
    vm_init(&virtual_machine, ss_functions);
    char* source_code = read_file(path);
    if (source_code == NULL)
        ss_throw("File '%s' could not be found\n", path);
    lex_Object object;
    lexObject_init(&object, source_code);
    lex(&object);
    if (flags.visualize_tokens) {
        if (object.token_used > 2500) {
            lex_free(&object);
            vm_free(&virtual_machine);
            free_and_null(source_code);
            ss_throw("Tokens too big to visualize\n");
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
    ReferenceTable reftable = init_reftable();
    compile(&s, &reftable);
    if (flags.preserve_bytecode) {
        char* bytecode_path = path;
        bytecode_path[strlen(bytecode_path)-1] = 'b';
        save_to_file(reftable.map->instructions, reftable.string_pool, reftable.map->length, bytecode_path);
    }
    free_ParseObject(&s);
    lex_free(&object);
    free_and_null(source_code);
    if (!flags.no_run) {
        int ret = vm_execute(&virtual_machine, reftable.string_pool, reftable.map->instructions, reftable.map->length);
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
        else if (!strcmp(argv[i], "--debug"))
            debug_env = true;
        else if (argv[i][0] == '-' && argv[i][1] == '-')
            ss_throw("Invalid flag '%s'\n", argv[i]);
    }
}


int main(int argc, char** argv) {
    CommandLineFlags flags = init_flags();
    read_flags(&flags, argc, argv);
    if (argc >= 2) {
        if (flags.check_version) {
            printf("V%s\n", VERSION);
        } else {
            char extension[255];
            get_extension(argv[1], extension);
            if (!strcmp(extension, BYTECODE_EXTENSION)) {
                if (!flags.is_view) {
                    return execute_bytecode(argv[1]);
                } else {
                    return visualize_bytecode(argv[1]);
                }
            } else if (!strcmp(extension, SOURCE_CODE_EXTENSION)) {
                return compile_and_run(flags, argv[1]);
            } else {
                ss_throw("Bad Argument: Invalid extension '%s'\n", extension);
            }
        }
    } else {
        shell_env = true;
        return run_shell();
    }
    return 0;
}
