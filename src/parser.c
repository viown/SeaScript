#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "lex.h"
#include "debug.h"

static size_t current_line = 1;

State parse_value(Token* token);

static inline bool is_keyword(Token* token, char* keyword) {
    return strcmp(token->value, keyword) == 0;
}

/* check whether current identifier leads to a function call */
static inline bool is_function_call(Token* current_token) {
    bool is_preceded = strcmp(NEXT_TOKEN(current_token).value, FUNC_OPEN) == 0;
    if (!IS_START_TOKEN(current_token)) {
        return is_preceded && (strcmp(PREVIOUS_TOKEN(current_token).value, "function") != 0);
    } else {
        return is_preceded;
    }
}

/* check whether current identifier is a function definition */
static inline bool is_function_definition(Token* current_token) {
    if (!IS_START_TOKEN(current_token)) {
        return strcmp(NEXT_TOKEN(current_token).value, FUNC_OPEN) == 0 && (strcmp(PREVIOUS_TOKEN(current_token).value, "function") == 0);
    } else {
        return false;
    }
}

static inline bool is_variable_declaration(Token* current_token) {
    if (!IS_START_TOKEN(current_token)) {
        return strcmp(PREVIOUS_TOKEN(current_token).value, "global") == 0;
    } else {
        return false;
    }
}

static inline bool is_variable_reassignment(Token* current_token) {
    bool is_assigning = current_token->token == IDENTIFIER && strcmp(NEXT_TOKEN(current_token).value, ASSIGNMENT) == 0;
    if (!IS_START_TOKEN(current_token)) {
        return is_assigning && strcmp(PREVIOUS_TOKEN(current_token).value, "global") != 0;
    } else {
        return is_assigning;
    }
}

static inline bool is_array_index(Token* current_token) {
    return current_token->token == IDENTIFIER && (strcmp(NEXT_TOKEN(current_token).value, INDEX_OPEN) == 0);
}

static inline bool is_array(Token* current_token) {
    return strcmp(current_token->value, ARRAY_OPEN) == 0 && strcmp(PREVIOUS_TOKEN(current_token).value, ASSIGNMENT) == 0;
}

static inline bool is_natural_identifier(Token* current_token) {
    return !is_array_index(current_token) && !is_function_call(current_token) && !is_function_definition(current_token);
}

void skip_to_end_call(Token** ptoken) {
    while (strcmp((*ptoken)->value, FUNC_CLOSE)) {
        (*ptoken)++;
        if (is_function_call(*ptoken)) {
            skip_to_end_call(ptoken);
            (*ptoken)++;
        }
    }
}

/* skips to end of statement */
void skip_to_end(Token** ptoken, const char* end) {
    while (strcmp((*ptoken)->value, end)) {
        if ((*ptoken)->token == NEWLINE) {
            current_line++;
        }
        (*ptoken)++;
    }
}

/* parses a function call's arguments */
void parse_function_call(State* state, Token* token) {
    Token* call_identifier = token;
    token += 2; /* skip '(' */
    ParseObject* arguments = (ParseObject*)ss_malloc(MAX_ARGUMENTS * sizeof(ParseObject));
    size_t length = 0;
    bool is_looping = true;
    while (is_looping) {
        ParseObject object;
        State* states = ss_malloc(255 * sizeof(State));
        int used = 0;
        /* TODO: Use parse_statement here instead */
        while (!is_eq(token->value, ARG_SEPARATOR)) {
            if (is_eq(token->value, FUNC_CLOSE)) {
                is_looping = false;
                break;
            } else if (token->token == NEWLINE) {
                ++token;
                continue;
            }
            State value = parse_value(token);
            if (value.type == s_FUNCTIONCALL) {
                skip_to_end_call(&token);
            }
            states[used++] = value;
            ++token;
        }
        object.length = used;
        object.states = states;
        arguments[length++] = object;
        ++token;
    }
    ss_FunctionCall* func_call = (ss_FunctionCall*)ss_malloc(sizeof(ss_FunctionCall));
    strcpy(func_call->function_name, call_identifier->value);
    func_call->arguments = arguments;
    func_call->arg_count = length;
    state->type = s_FUNCTIONCALL;
    state->state = func_call;
}

ArgumentCollection parse_function_arguments(Token** ptoken) {
    ArgumentCollection collection;
    if (strcmp((*ptoken)->value, FUNC_OPEN) == 0)
        (*ptoken)++; /* skip '(' */
    ss_Identifier* arguments = (ss_Identifier*)ss_malloc(MAX_ARGUMENTS * sizeof(ss_Identifier));
    int used = 0;
    while (strcmp((*ptoken)->value, FUNC_CLOSE)) {
        if ((*ptoken)->token == IDENTIFIER) {
            ss_Identifier arg;
            strcpy(arg.identifier, (*ptoken)->value);
            arguments[used++] = arg;
        } else if (strcmp((*ptoken)->value, ARG_SEPARATOR) != 0) {
            ss_throw("line %lu: Invalid token in function arguments '%s'", current_line, (*ptoken)->value);
        }
        (*ptoken)++;
    }
    (*ptoken)++; /* move to SCOPE_OPEN */
    if (strcmp((*ptoken)->value, SCOPE_OPEN) != 0) {
        ss_throw("line %lu: Function doesn't lead to scope; missing '%s'", current_line, SCOPE_OPEN);
    }

    collection.length = used;
    collection.arguments = arguments;
    return collection;
}

lex_Object collect_tokens_from_scope(Token** ptoken) {
    lex_Object object;
    object.tokens = (Token*)ss_malloc(1000 * sizeof(Token));
    object.token_size = 1000;
    object.token_used = 0;
    if (strcmp((*ptoken)->value, SCOPE_OPEN) == 0)
        (*ptoken)++; /* skip '{' */
    int function_nest = 0;
    while (!((*ptoken)->is_end)) {
        object.tokens[object.token_used++] = **ptoken;
        if (strcmp((*ptoken)->value, SCOPE_OPEN) == 0) {
            function_nest++;
        } else if (strcmp((*ptoken)->value, SCOPE_CLOSE) == 0) {
            if (function_nest != 0) {
                function_nest--;
            } else {
                break;
            }
        }
        if (object.token_used == object.token_size) {
            object.token_size *= 2;
            object.tokens = (Token*)realloc(object.tokens, object.token_size * sizeof(Token));
        }
        if ((*ptoken)->token == NEWLINE)
            current_line++;
        (*ptoken)++;
    }
    object.tokens[0].is_start = true;
    object.tokens[object.token_used-1].is_end = true;
    return object;
}

State parse_function_definition(Token** current_token) {
    /* TODO: Implement lambda support 'function foo(a, b) = a + b;' */
    ss_Function* function = (ss_Function*)ss_malloc(sizeof(ss_Function));
    Token* function_name = ++(*current_token);
    ++(*current_token); /* move to '(' */
    ArgumentCollection arguments = parse_function_arguments(current_token);
    lex_Object scopeTokens = collect_tokens_from_scope(current_token);
    ParseObject scope = parse(scopeTokens);
    free_and_null(scopeTokens.tokens);
    strcpy(function->function_name, function_name->value);
    function->arguments = arguments;
    function->scope = scope;
    function->is_lamda = false;
    State state = {function, s_FUNCTION};
    return state;
}

void parse_literal(State* state, Token* token) {
    ss_Literal* literal = (ss_Literal*)ss_malloc(sizeof(ss_Literal));
    state->type = s_LITERAL;
    if (token->token == ILITERAL) {
        double* value = (double*)ss_malloc(sizeof(double));
        *value = atof(token->value);
        literal->value = value;
        literal->type = l_INTEGER;
    } else if (token->token == SLITERAL) {
        char* literal_string = (char*)ss_malloc(strlen(token->value) + 1);
        strcpy(literal_string, token->value);
        literal->value = literal_string;
        literal->type = l_STRING;
    }
    state->state = literal;
}

void parse_identifier(State* state, Token* token) {
    ss_Identifier* identifier = (ss_Identifier*)ss_malloc(sizeof(ss_Identifier));
    strcpy(identifier->identifier, token->value);
    state->state = identifier;
    state->type = s_IDENTIFIER;
}

void parse_operator(State* state, Token* token) {
    if (is_math_op(token->value[0])) {
        ss_Operator* op = (ss_Operator*)ss_malloc(sizeof(ss_Operator));
        Operator* math_operator = (char*)ss_malloc(3);
        strcpy(math_operator, token->value);
        op->type = MATH;
        op->op = math_operator;
        state->state = op;
        state->type = s_OPERATOR;
    } else if (is_comparison_op(token->value)) {
        /* incomplete */
        ss_Operator* op = (ss_Operator*)ss_malloc(sizeof(ss_Operator));
        Operator* comparison_op = (char*)ss_malloc(3);
        strcpy(comparison_op, token->value);
        op->type = COMPARISON;
        op->op = comparison_op;
        state->state = op;
        state->type = s_OPERATOR;
    }
}

void parse_array_index(State* state, Token* token) {
    Token* name = token;
    token += 2;
    State* states = (State*)ss_malloc(255 * sizeof(State));
    int used = 0;
    while (strcmp(token->value, INDEX_CLOSE) != 0) {
        states[used++] = parse_value(token);
        if (is_function_call(token)) {
            skip_to_end_call(&token);
        }
        token++;
    }
    ss_IndexOperator* op = (ss_IndexOperator*)ss_malloc(sizeof(ss_IndexOperator));
    ParseObject* parse_object = (ParseObject*)ss_malloc(sizeof(ParseObject));
    parse_object->length = used;
    parse_object->states = states;
    op->name = name->value;
    op->parse_object = parse_object;
    state->type = s_INDEX;
    state->state = op;
}

State parse_value(Token* token) {
    State state;
    if (is_function_call(token)) {
        parse_function_call(&state, token);
    } else if (is_array_index(token)) {
        parse_array_index(&state, token);
    } else if (is_literal(token->token)) {
        parse_literal(&state, token);
    } else if (token->token == IDENTIFIER) {
        parse_identifier(&state, token);
    } else if (token->token == OPERATOR) {
        parse_operator(&state, token);
    } else {
        ss_unreachable("Unidentified token passed to parse_value: %s", token->value);
    }
    return state;
}

/* reads the tokens until an ';' is encountered */
ParseObject parse_statement(Token* token, const char* end) {
    ParseObject object;
    State* states = (State*)ss_malloc(255 * sizeof(State));
    int used = 0;
    while (strcmp(token->value, end) != 0 && !token->is_end) {
        states[used++] = parse_value(token);
        if (is_function_call(token)) {
            skip_to_end_call(&token);
        } else if (is_array_index(token)) {
            skip_to_end(&token, INDEX_CLOSE);
        }
        token++;
        while (token->token == NEWLINE) {
            token++;
        }
    }
    object.states = states;
    object.length = used;
    return object;
}

ss_Variable* create_variable(char* var_name, ParseObject var_states) {
    ss_Variable* variable = (ss_Variable*)ss_malloc(sizeof(ss_Variable));
    strcpy(variable->variable_name, var_name);
    variable->states = var_states;
    variable->is_initialized = true;
    return variable;
}

/* create a reassignment operation */
ss_Reassignment* create_reassignment(char* var_name, ParseObject var_states) {
    ss_Reassignment* reassignment = (ss_Reassignment*)ss_malloc(sizeof(ss_Reassignment));
    strcpy(reassignment->variable_name, var_name);
    reassignment->states = var_states;
    return reassignment;
}

State parse_variable_declaration(Token* current_token, State* states, size_t length) {
    Token* variable_name = ++current_token; /* name of variable */
    if (variable_name->token == KEYWORD) {
        ss_throw("line %lu: variable name cannot be a reserved keyword", current_line);
    } else if (variable_declared(variable_name, states, length)) {
        ss_throw("line: %lu: attempted redeclaration of variable '%s'\n", current_line, variable_name->value);
    }
    State variable_state;
    if (is_eq(NEXT_TOKEN(current_token).value, EOS)) { /* if declared but uninitialized */
        ss_Variable* variable = (ss_Variable*)ss_malloc(sizeof(ss_Variable));
        strcpy(variable->variable_name, variable_name->value);
        variable->is_initialized = false;

        variable_state.state = variable;
        variable_state.type = s_VARIABLE;
    } else {
        ++current_token; /* skip assignment operator */
        Token* value = ++current_token; /* pointer to first token */
        ParseObject var_state = parse_statement(value, EOS);
        ss_Variable* variable = create_variable(variable_name->value, var_state);

        variable_state.state = variable;
        variable_state.type = s_VARIABLE;
    }
    return variable_state;
}

State parse_variable_reassignment(Token* current_token, State* states, size_t length) {
    Token* variable_name = current_token;
    if (!IS_START_TOKEN(variable_name) && PREVIOUS_TOKEN(variable_name).token == IDENTIFIER) {
        ss_throw("line %lu: unknown keyword '%s'\n", current_line, PREVIOUS_TOKEN(variable_name).value);
    }
    ++current_token;
    Token* value = ++current_token;
    ParseObject var_state = parse_statement(value, EOS);
    ss_Reassignment* reassignment = create_reassignment(variable_name->value, var_state);

    State reassignment_state = {reassignment, s_REASSIGN};
    return reassignment_state;
}

bool variable_declared(Token* token, State* states, int length) {
    for (int i = 0; i < length; ++i) {
        State state = states[i];
        if (state.type == s_VARIABLE) {
            ss_Variable variable = get_variable(state.state);
            if (strcmp(variable.variable_name, token->value) == 0) {
                return true;
            }
        }
    }
    return false;
}


/* parses an entire scope */
ParseObject parse(lex_Object object) {
    Token* current_token = object.tokens;
    Token* end = &current_token[object.token_used];
    ParseObject parse_obj;
    size_t size = 1000;
    size_t length = 0;
    State* states = (State*)ss_malloc(size * sizeof(State));
    while (current_token != end) {
        if (current_token->token == NEWLINE) {
            current_line++;
        } else if (current_token->token == KEYWORD) {
            if (is_variable_declaration(current_token + 1)) { /* variable declaration? */
                State variable_state = parse_variable_declaration(current_token, states, length);
                states[length++] = variable_state;
                skip_to_end(&current_token, EOS);
            } else if (is_function_definition(current_token + 1)) {
                states[length++] = parse_function_definition(&current_token);
            } else if (is_keyword(current_token, "return")) {
                ss_ReturnStatement* return_statement = (ss_ReturnStatement*)ss_malloc(sizeof(ss_ReturnStatement));
                ParseObject statement = parse_statement(current_token+1, EOS);
                *return_statement = (ss_ReturnStatement)statement;
                State state = {return_statement, s_RETURN};
                states[length++] = state;
            } else if (is_keyword(current_token, "if")) {
                ss_IfStatement* if_statement = (ss_IfStatement*)ss_malloc(sizeof(ss_IfStatement));
                if_statement->condition = parse_statement(++current_token, "{");
                skip_to_end(&current_token, "{");
                lex_Object data = collect_tokens_from_scope(&current_token);
                data.token_used--;
                if (data.token_used > 0) {
                    data.tokens[data.token_used - 1].is_end = true;
                    ParseObject* obj = malloc(sizeof(ParseObject));
                    *obj = parse(data);
                    if_statement->scope = obj;
                } else {
                    if_statement->scope = NULL;
                }
                State state = {if_statement, s_IFSTATEMENT};
                states[length++] = state;
                free_and_null(data.tokens);
            }
        } else if (current_token->token == IDENTIFIER) {
            if (is_function_call(current_token)) {
                states[length++] = parse_value(current_token);
                skip_to_end(&current_token, EOS);
            } else if (is_variable_reassignment(current_token)) {
                State variable_state = parse_variable_reassignment(current_token, states, length);
                states[length++] = variable_state;
                skip_to_end(&current_token, EOS);
            } else if (is_array_index(current_token)) {
                states[length++] = parse_value(current_token);
                skip_to_end(&current_token, INDEX_CLOSE);
            }
        }
        if (length >= size) {
            size *= 2;
            states = (State*)realloc(states, size * sizeof(State));
        }
        current_token++;
    }
    parse_obj.states = states;
    parse_obj.length = length;
    return parse_obj;
}

void free_state(State* state) {
    if (state->type == s_IDENTIFIER) {
        free_and_null(state->state);
    } else if (state->type == s_LITERAL) {
        ss_Literal literal = get_literal(state->state);
        free_and_null(literal.value);
        free_and_null(state->state);
    } else if (state->type == s_OPERATOR) {
        ss_Operator obj_op = get_operator(state->state);
        if (obj_op.type == MATH || obj_op.type == COMPARISON) {
            free_and_null(obj_op.op);
            free_and_null(state->state);
        }
    } else if (state->type == s_FUNCTIONCALL) {
        ss_FunctionCall call = get_functioncall(state->state);
        for (int i = 0; i < call.arg_count; ++i) {
            ParseObject argument = call.arguments[i];
            for (int j = 0; j < argument.length; ++j) {
                free_state(&argument.states[j]);
            }
            free_and_null(argument.states);
        }
        free_and_null(call.arguments);
        free_and_null(state->state);
    } else if (state->type == s_INDEX) {
        ss_IndexOperator op = get_index(state->state);
        for (size_t i = 0; i < op.parse_object->length; ++i) {
            free_state(&op.parse_object->states[i]);
        }
        free_and_null(op.parse_object->states);
        free_and_null(op.parse_object);
        free_and_null(state->state);
    } else if (state->type == s_IFSTATEMENT) {
        ss_IfStatement if_statement = get_ifstatement(state->state);
        free_and_null(if_statement.condition.states);
        free_ParseObject(if_statement.scope);
        free_and_null(if_statement.scope);
        free_and_null(state->state);
    } else if (state->type == s_FUNCTION) {
        ss_Function function = get_function(state->state);
        free_and_null(function.arguments.arguments);
        free_ParseObject(&function.scope);
        free_and_null(state->state);
    }
}

void free_ParseObject(ParseObject* object) {
    for (size_t i = 0; i < object->length; ++i) {
        State current = object->states[i];
        if (current.type == s_VARIABLE) {
            ss_Variable var = get_variable(current.state);
            ParseObject states = var.states;
            if (var.is_initialized) {
                for (int i = 0; i < states.length; ++i) {
                    free_state(&states.states[i]);
                }
                free_and_null(states.states);
            }
            free_and_null(current.state);
        } else if (current.type == s_REASSIGN) {
            ss_Reassignment reassignment = get_reassignment(current.state);
            ParseObject states = reassignment.states;
            for (int i = 0; i < states.length; ++i) {
                free_state(&states.states[i]);
            }
            free_and_null(states.states);
            free_and_null(current.state);
        } else if (current.type == s_FUNCTIONCALL) {
            free_state(&current);
        } else if (current.type == s_FUNCTION) {
            free_state(&current);
        }
    }
    free_and_null(object->states);
}
