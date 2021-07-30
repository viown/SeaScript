#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include "vm.h"
#include "parser.h"
#include "lex.h"
#include "ssfunctions.h"
#include "shell.h"
#include "debug.h"

/*
** TODO: Create a reusable shell environment
*/

static const char* startup = "SeaScript Interactive Shell v1.0\nType :help for help\n";

ShellEnvironment* create_shell_environment() {
    ShellEnvironment* shell_environment = ss_malloc(sizeof(ShellEnvironment));
    shell_environment->reftable = init_reftable();
    shell_environment->shell_active = true;
    shell_environment->parse_objects = ss_malloc(5 * sizeof(ParseObject));
    shell_environment->parse_objects_size = 5;
    shell_environment->parse_objects_length = 0;
    return shell_environment;
}

void push_parse_object(ShellEnvironment* env, ParseObject obj) {
    if (env->parse_objects_length == env->parse_objects_size) {
        env->parse_objects_size += 5;
        env->parse_objects = (ParseObject*)realloc(env->parse_objects, env->parse_objects_size * sizeof(ParseObject));
    }
    env->parse_objects[env->parse_objects_length++] = obj;
}

void print_start() {
    printf("%s", startup);
}

char* read_line() {
    char* s = malloc(1000);
    fgets(s, 1000, stdin);
    s[strlen(s)-1] = '\0';
    return s;
}

void terminate_shell(ShellEnvironment* env) {
    env->shell_active = false;
    reftable_free(&env->reftable);
    free_and_null(env->parse_objects);
    free_and_null(env);
}

void execute_shell_command(ShellEnvironment** env, const char* cmd) {
    if (!strcmp(cmd, ":q")) {
        (*env)->shell_active = false;
    } else if (!strcmp(cmd, ":clear")) {
#ifdef __linux__
        system("clear");
#else
        system("cls");
#endif // __linux__
        print_start();
    } else if (!strcmp(cmd, ":killenv")) {
        terminate_shell(*env);
        ShellEnvironment* new_shellenv = create_shell_environment();
        *env = new_shellenv;
    } else if (!strcmp(cmd, ":help")) {
        printf("%s", help);
    } else {
        printf("Unknown command, type :help for a list of commands\n");
    }
}

void execute_input(ShellEnvironment* env, VirtualMachine* vm, char* input) {
    lex_Object object;
    lexObject_init(&object, input);
    lex(&object);
    ParseObject parse_data = parse(object);
    compile(&parse_data, &env->reftable);
    vm_execute(vm, env->reftable.map->instructions, env->reftable.map->length);
    push_parse_object(env, parse_data);
    lex_free(&object);
}

int run_shell() {
    ShellEnvironment* environment = create_shell_environment();
    print_start();
    VirtualMachine vm;
    vm_init(&vm, ss_functions);
    while (environment->shell_active) {
        printf("> ");
        char* input = read_line();
        if (input[0] == ':') {
            /* shell specific command */
            execute_shell_command(&environment, input);
        } else {
            execute_input(environment, &vm, input);
        }
        free(input);
    }
    vm_free(&vm);
    for (int i = 0; i < environment->parse_objects_length; ++i) {
        free_ParseObject(&environment->parse_objects[i]);
    }
    terminate_shell(environment);
    printf("Successfully exited\n");
    return 0;
}
