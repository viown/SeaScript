#ifndef SS_FUNCTIONS_H
#define SS_FUNCTIONS_H
#include "./seavm/stack.h"
#include "./seavm/vm.h"

void ss_f_print(VirtualMachine* vm);
void ss_f_input(VirtualMachine* vm);
void ss_f_exit(VirtualMachine* vm);

static const ss_BaseFunction ss_functions[] = {
    {"print", &ss_f_print, 1},
    {"input", &ss_f_input, 0},
    {"exit", &ss_f_exit, 1},
};
int lookup_global_function(char* func_name);

#endif // SS_FUNCTIONS_H
