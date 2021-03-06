#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <readline/readline.h>
#include "stack.h"
#include "ssfunctions.h"

#define return_null(stack) \
    push_stack(stack, create_bool(0));

#define return_string(stack, str) \
    push_stack(stack, create_string(str));

#define return_number(stack, num) \
    push_stack(stack, create_number(num))

int lookup_global_function(char* func_name) {
    for (int i = 0; i < sizeof(ss_functions) / sizeof(ss_functions[0]); ++i) {
        if (!strcmp(ss_functions[i].name, func_name))
            return i;
    }
    return -1;
}

void ss_f_print(VirtualMachine* vm) {
    StackObject obj = pop_stack(&vm->stack[vm->sp]);
    switch (obj.type) {
    case BOOL:
        if (obj.object.m_bool == 0)
            printf("false");
        else
            printf("true");
        break;
    case NUMBER:
        if ((obj.object.m_number - (int64_t)obj.object.m_number) == 0) {
            printf("%lld", (long long int)obj.object.m_number);
        } else {
            printf("%g", obj.object.m_number);
        }
        break;
    case STRING:
        printf("%s", obj.object.m_string);
        break;
    default:
        exit(-1);
    }
    printf("\n");
    return_null(&vm->stack[vm->sp]);
}

void ss_f_input(VirtualMachine* vm) {
    /* temporary test for strings */
    char* input = readline("");
    return_string(&vm->stack[vm->sp], input);
}

void ss_f_exit(VirtualMachine* vm) {
    StackObject* top = top_stack(&vm->stack[vm->sp]);
    if (top->type == NUMBER) {
        exit((unsigned char)top->object.m_number);
    } else {
        exit(top->object.m_bool);
    }
}

void ss_f_to_string(VirtualMachine* vm) {
    StackObject number = pop_stack(&vm->stack[vm->sp]);
    char* num_str = malloc(20);
    push_heap_object(vm, num_str); // TODO: Stop using this garbage and place num_str inside the virtual machine's memory.
    sprintf(num_str, "%d", (int32_t)number.object.m_number);
    return_string(&vm->stack[vm->sp], num_str);
}

void ss_f_to_number(VirtualMachine* vm) {
    StackObject str = pop_stack(&vm->stack[vm->sp]);
    if (str.type != STRING) {
        if (str.type == NUMBER) {
            return_number(&vm->stack[vm->sp], str.object.m_number);
        } else {
            return_null(&vm->stack[vm->sp]);
        }
    } else {
        int64_t num = 0;
        t_string s = str.object.m_string;
        for (int i = 0; i < strlen(s); ++i) {
            int n = s[i] - '0';
            num = num * 10 + n;
        }
        return_number(&vm->stack[vm->sp], (double)num);
    }
}

void ss_f_time(VirtualMachine* vm) {
    StackObject current_time;
    current_time.object.m_number = (double)time(NULL);
    current_time.type = NUMBER;
    push_stack(&vm->stack[vm->sp], current_time);
}


