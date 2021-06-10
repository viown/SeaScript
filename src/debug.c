#include <stdio.h>
#include "./compiler.h" // for IS_INT
#include "./debug.h"

void visualize_tokens(lex_Object* object) {
    for (int i = 0; i < object->token_used; ++i) {
        printf("<SPECIFIER='%d', VALUE='%s', IS_START=%d, IS_END=%d>\n", object->tokens[i].token, object->tokens[i].value, object->tokens[i].is_start, object->tokens[i].is_end);
    }
}

void read_arguments(ss_FunctionCall fcall, bool is_nested_call) {
    printf("%s(", fcall.function_name);
    for (size_t i = 0; i < fcall.arg_count; ++i) {
        State argument = fcall.arguments[i];
        if (argument.type == s_LITERAL) {
            ss_Literal arg_value = get_literal(argument.state);
            if (arg_value.type == l_INTEGER) {
                if (i + 1 == fcall.arg_count) {
                    printf("%f", arg_value.value);
                } else {
                    printf("%f, ", arg_value.value);
                }
            }
        } else if (argument.type == s_FUNCTIONCALL) { /* function call as argument */
            ss_FunctionCall arg_call = get_functioncall(argument.state);
            read_arguments(arg_call, true);
        }
    }
    printf(")");
}

void print_variable(void* variable, StateType type) {
    ss_Variable var = get_variable(variable);
    if (type == s_REASSIGN) {
        printf("%s = ", var.variable_name);
    } else {
        printf("global %s = ", var.variable_name);
    }
    for (int i = 0; i < var.states.length; ++i) {
        State st = var.states.states[i];
        if (st.type == s_IDENTIFIER) {
            printf("%s ", get_identifier(st.state).identifier);
        } else if (st.type == s_OPERATOR) {
            ss_Operator s_OP = get_operator(st.state);
            Operator op = *(Operator*)s_OP.op;
            printf("%c ", op);
        } else if (st.type == s_LITERAL) {
            ss_Literal literal = get_literal(st.state);
            if (literal.type == l_INTEGER) {
                if (IS_INT(literal.value)) {
                    int temp = (int)literal.value;
                    printf("%d ", temp);
                } else {
                    printf("%f ", literal.value);
                }
            }
        } else {
            printf("<unknown> ");
        }
    }
    printf("\n");
}

/* visualizes the parse object into a tree-like structure */
void visualize_states(ParseObject* object) {
    for (int i = 0; i < object->length; ++i) {
        State current = object->states[i];
        if (current.type == s_VARIABLE) {
            print_variable(current.state, s_VARIABLE);
        } else if (current.type == s_REASSIGN) {
            print_variable(current.state, s_REASSIGN);
        } else if (current.type == s_FUNCTIONCALL) {
            ss_FunctionCall fcall = get_functioncall(current.state);
            read_arguments(fcall, false);
            printf("\n");
        }
    }
    printf("\n");
}

void visualize_token(Token* token) {
    printf("<SPECIFIER=%d, VALUE=%s>\n", token->token, token->value);
}
