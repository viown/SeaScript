#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "./ssfunctions.h"

int lookup_global_function(char* func_name) {
    for (int i = 0; i < sizeof(ss_functions) / sizeof(ss_functions[0]); ++i) {
        if (!strcmp(ss_functions[i].name, func_name))
            return i;
    }
    return -1;
}

void ss_f_print(VirtualMachine* vm) {
    StackObject obj = pop_stack(&vm->stack);
    switch (obj.type) {
    case BOOL:
        printf("%d", obj.object.m_bool);
        break;
    case INT32:
        printf("%d", obj.object.m_int32);
        break;
    case INT64:
        printf("%lld", obj.object.m_int64);
        break;
    case DOUBLE:
        printf("%f", obj.object.m_double);
        break;
    case STRING:
        printf("%s", obj.object.m_string);
        break;
    default:
        exit(-1);
    }
    printf("\n");
}

void ss_f_input(VirtualMachine* vm) {

}

void ss_f_exit(VirtualMachine* vm) {
    StackObject* top = top_stack(&vm->stack);
    exit(top->object.m_int32);
}
