#include <stdlib.h>
#include "./ssfunctions.h"

void ss_f_exit(Vm* vm) {
    StackObject* exit_code = top_stack(&vm->stack);
    exit(*(int32_t*)exit_code->object);
}
