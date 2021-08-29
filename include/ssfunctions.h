#ifndef SS_FUNCTIONS_H
#define SS_FUNCTIONS_H
#include "seavm/stack.h"
#include "seavm/vm.h"

void ss_f_print(VirtualMachine* vm);
void ss_f_input(VirtualMachine* vm);
void ss_f_exit(VirtualMachine* vm);
void ss_f_to_string(VirtualMachine* vm);
void ss_f_to_number(VirtualMachine* vm);
void ss_f_time(VirtualMachine* vm);

static const ss_BaseFunction ss_functions[] = {
    {"print", &ss_f_print, 1},
    {"input", &ss_f_input, 0},
    {"exit", &ss_f_exit, 1},
    {"to_string", &ss_f_to_string, 1},
    {"to_number", &ss_f_to_number, 1},
    {"time", &ss_f_time, 0},
};
int lookup_global_function(char* func_name);

#endif // SS_FUNCTIONS_H
