#ifndef SS_SHELL_H
#define SS_SHELL_H
#include "vm.h"
#include "compiler.h"

static const char* startup = "SeaScript Interactive Shell v1.1\nType :help for help\n";
static const char* const help = ":help - Shows this list\n:clear - Clears the console\n:killenv - Kills the current shell environment and replaces it with a new one\n:q - exit\n";

/*
* A shell environment stores data such as previous variable/function references
* These references should be available on the virtual machine's memory
*/
struct ShellEnvironment {
    VirtualMachine vm;
    bool shell_active;
    ReferenceTable reftable;
    ParseObject* parse_objects;
    size_t parse_objects_length;
    size_t parse_objects_size;
};

typedef struct ShellEnvironment ShellEnvironment;

int run_shell();

#endif // SS_SHELL_H
