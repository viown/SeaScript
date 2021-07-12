#include <stdlib.h>
#include "./ssfunctions.h"

void ss_f_exit(VirtualMachine* vm) {
    StackObject* top = top_stack(&vm->stack);
    exit(top->object.m_int32);
}
