#ifndef SS_FUNCTIONS_H
#define SS_FUNCTIONS_H
#include "./seavm/stack.h"
#include "./seavm/vm.h"

void ss_f_exit(VirtualMachine* vm);
void ss_f_abs(VirtualMachine* vm);

static const ss_BaseFunction ss_functions[] = {
    {"exit", &ss_f_exit},
};

#endif // SS_FUNCTIONS_H
