#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "./compiler.h" // for IS_INT
#include "./debug.h"

void ss_throw(const char* error, ...) {
    va_list args;
    va_start(args, error);
    vprintf(error, args);
    va_end(args);
    exit(1);
}

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
                double value = load_literal(arg_value);
                if (i + 1 == fcall.arg_count) {
                    printf("%f", value);
                } else {
                    printf("%f, ", value);
                }
            } else if (arg_value.type == l_STRING) {
                char* str = (char*)arg_value.value;
                if (i + 1 == fcall.arg_count) {
                    printf("%s", str);
                } else {
                    printf("%s, ", str);
                }
            }
        } else if (argument.type == s_FUNCTIONCALL) { /* function call as argument */
            ss_FunctionCall arg_call = get_functioncall(argument.state);
            read_arguments(arg_call, true);
        } else if (argument.type == s_IDENTIFIER) {
            ss_Identifier identifier = get_identifier(argument.state);
            if (i + 1 == fcall.arg_count) {
                printf("%s", identifier.identifier);
            } else {
                printf("%s, ", identifier.identifier);
            }
        }
    }
    printf(")");
}

void print_variable(void* variable, StateType type) {
    ss_Variable var = get_variable(variable);
    if (var.is_initialized) {
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
                    double value = load_literal(literal);
                    if (literal.type == l_INTEGER) {
                        printf("%f ", value);
                    }
                } else if (literal.type == l_STRING) {
                    char* str = (char*)literal.value;
                    printf("%s", str);
                }
            } else if (st.type == s_FUNCTIONCALL) {
                ss_FunctionCall fcall = get_functioncall(st.state);
                read_arguments(fcall, false);
                printf(" ");
            } else if (st.type == s_INDEX) {
                ss_IndexOperator index_operator = get_index(st.state);
                printf("%s[...] ", index_operator.name);
            } else {
                printf("<unknown> ");
            }
        }
    } else {
        printf("global %s; /* uninitialized */", var.variable_name);
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
