#include <stdlib.h>
#include "./ssfunctions.h"

void ss_f_exit(Stack* stack) {
    stack_type* exit_code = top_stack(stack);
    exit(*exit_code);
}

void ss_f_abs(Stack* stack) {
    stack_type* num = top_stack(stack);
    if (*num < 0) {
        push_stack(stack, -(*num));
    } else {
        push_stack(stack, *num);
    }
}
