#include <stdlib.h>
#include "./ssfunctions.h"

void ss_f_exit(Vm* vm) {
    stack_type* exit_code = top_stack(&vm->stack);
    exit(*exit_code);
}
