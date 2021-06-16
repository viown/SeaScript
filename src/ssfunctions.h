#ifndef SS_FUNCTIONS_H
#define SS_FUNCTIONS_H
#include "./seavm/stack.h"
#include "./seavm/vm.h"

void ss_f_exit(Stack* stack);
void ss_f_abs(Stack* stack);

static const ss_BaseFunction ss_functions[] = {
    {"exit", &ss_f_exit},
    {"abs", &ss_f_abs},
};

#endif // SS_FUNCTIONS_H
