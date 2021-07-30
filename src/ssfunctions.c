#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "stack.h"
#include "ssfunctions.h"

#define return_null(stack) \
    push_stack(stack, create_bool(0));

#define return_string(stack, str) \
    push_stack(stack, create_string(str));

#define return_int32(stack, num) \
    push_stack(stack, create_int32(num))

#define return_int64(stack, num) \
    push_stack(stack, create_int64(num))

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
        if (obj.object.m_bool == 0)
            printf("false");
        else
            printf("true");
        break;
    case INT32:
        printf("%d", obj.object.m_int32);
        break;
    case INT64:
        printf("%lld", (long long int)obj.object.m_int64);
        break;
    case DOUBLE:
        printf("%g", obj.object.m_double);
        break;
    case STRING:
        printf("%s", obj.object.m_string);
        break;
    default:
        exit(-1);
    }
    printf("\n");
    return_null(&vm->stack);
}

void ss_f_input(VirtualMachine* vm) {
    /* temporary test for strings */
    char* input = malloc(50);
    push_heap_object(vm, input); // mark for free
    fgets(input, 50, stdin);
    input[strlen(input)-1] = '\0';
    return_string(&vm->stack, input);
}

void ss_f_exit(VirtualMachine* vm) {
    StackObject* top = top_stack(&vm->stack);
    exit(top->object.m_int32);
}

void ss_f_test_add(VirtualMachine* vm) {
    /* function for testing return values */
    StackObject num1 = pop_stack(&vm->stack);
    StackObject num2 = pop_stack(&vm->stack);
    StackObject sum;
    sum.type = INT64;
    sum.object.m_int64 = num1.object.m_int32 + num2.object.m_int32;
    push_stack(&vm->stack, sum); /* return sum */
}

void ss_f_to_string(VirtualMachine* vm) {
    StackObject number = pop_stack(&vm->stack);
    char* num_str = malloc(20);
    push_heap_object(vm, num_str);
    sprintf(num_str, "%d", number.object.m_int32);
    return_string(&vm->stack, num_str);
}

void ss_f_to_number(VirtualMachine* vm) {
    StackObject str = pop_stack(&vm->stack);
    if (str.type != STRING) {
        if (str.type == INT32) {
            return_int32(&vm->stack, str.object.m_int32);
        } else if (str.type == INT64) {
            return_int64(&vm->stack, str.object.m_int64);
        } else {
            return_null(&vm->stack);
        }
    } else {
        int64_t num = 0;
        t_string s = str.object.m_string;
        for (int i = 0; i < strlen(s); ++i) {
            int n = s[i] - '0';
            num = num * 10 + n;
        }
        return_int64(&vm->stack, num);
    }
}

void ss_f_time(VirtualMachine* vm) {
    StackObject current_time;
    current_time.object.m_int64 = (int64_t)time(NULL);
    current_time.type = INT64;
    push_stack(&vm->stack, current_time);
}


