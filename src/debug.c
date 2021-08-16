#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "compiler.h" // for IS_INT
#include "debug.h"

void print_state(State st);

void* ss_malloc(size_t size) {
    void* mem = malloc(size);
    if (mem == NULL) {
        ss_throw("fatal: memory allocation failed, program halted.\n");
    }
    return mem;
}

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
    ParseObject* arguments = fcall.arguments;
    for (size_t i = 0; i < fcall.arg_count; ++i) {
        ParseObject argument = arguments[i];
        for (int j = 0; j < argument.length; ++j) {
            print_state(argument.states[j]);
        }
        printf(",");
    }
    if (is_nested_call)
        printf("), ");
    else
        printf(")\n");
}

void print_state(State st) {
    if (st.type == s_IDENTIFIER) {
        printf("%s", get_identifier(st.state).identifier);
    } else if (st.type == s_OPERATOR) {
        ss_Operator s_OP = get_operator(st.state);
        char* op = (char*)s_OP.op;
        printf("%s", op);
    } else if (st.type == s_LITERAL) {
        ss_Literal literal = get_literal(st.state);
        if (literal.type == l_INTEGER) {
            double value = load_literal(literal);
            if (literal.type == l_INTEGER) {
                printf("%g", value);
            }
        } else if (literal.type == l_STRING) {
            char* str = (char*)literal.value;
            printf("\"%s\"", str);
        }
    } else if (st.type == s_FUNCTIONCALL) {
        ss_FunctionCall fcall = get_functioncall(st.state);
        read_arguments(fcall, false);
        printf(" ");
    } else if (st.type == s_INDEX) {
        ss_IndexOperator index_operator = get_index(st.state);
        ParseObject* obj = index_operator.parse_object;
        printf("%s[", index_operator.name);
        for (int i = 0; i < obj->length; ++i) {
            State st = obj->states[i];
            print_state(st);
        }
        printf("] ");
    } else {
        printf("<unknown> ");
    }
}

void print_variable(void* variable) {
    ss_Variable var = get_variable(variable);
    if (var.is_initialized) {
        printf("global %s = ", var.variable_name);
        for (int i = 0; i < var.states.length; ++i) {
            State st = var.states.states[i];
            print_state(st);
        }
    } else {
        printf("global %s; /* uninitialized */", var.variable_name);
    }
    printf("\n");
}

void print_reassignment(void* reassignment) {
    ss_Reassignment reassign = get_reassignment(reassignment);
    printf("%s = ", reassign.variable_name);
    for (int i = 0; i < reassign.states.length; ++i) {
        State st = reassign.states.states[i];
        print_state(st);
    }
    printf("\n");
}

/* visualizes the parse object into a tree-like structure */
void visualize_states(ParseObject* object) {
    for (int i = 0; i < object->length; ++i) {
        State current = object->states[i];
        if (current.type == s_VARIABLE) {
            print_variable(current.state);
        } else if (current.type == s_REASSIGN) {
            print_reassignment(current.state);
        } else if (current.type == s_FUNCTIONCALL) {
            ss_FunctionCall fcall = get_functioncall(current.state);
            read_arguments(fcall, false);
            printf("\n");
        } else if (current.type == s_RETURN) {
            printf("return ");
            ss_ReturnStatement return_states = get_return(current.state);
            for (int i = 0; i < return_states.length; ++i) {
                State st = return_states.states[i];
                print_state(st);
            }
            printf("\n");
        } else if (current.type == s_FUNCTION) {
            ss_Function function = get_function(current.state);
            printf("function %s(", function.function_name);
            for (int i = 0; i < function.arguments.length; ++i) {
                if ((i+1) == function.arguments.length)
                    printf("%s", function.arguments.arguments[i].identifier);
                else
                    printf("%s, ", function.arguments.arguments[i].identifier);
            }
            printf(") {\n");
            visualize_states(&function.scope);
            printf("}\n");
        }
    }
    printf("\n");
}

void visualize_token(Token* token) {
    printf("<SPECIFIER=%d, VALUE=%s>\n", token->token, token->value);
}
