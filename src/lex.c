#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./lex.h"

bool is_operator(char c) {
    switch (c) {
    case ',':
    case '>':
    case '<':
    case '+':
    case '-':
    case '*':
    case '/':
    case '=':
    case ';': /* optional; but can be used to indicate the end of line. */
        return true;
    default:
        return false;
    }
}

bool is_punctuator(char c) {
    switch (c) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
        return true;
    default:
        return false;
    }
}

bool is_keyword(const char* c) {
    for (size_t i = 0; i < keyword_count; ++i) {
        if (strcmp(ss_keywords[i], c) == 0) {
            return true;
        }
    }
    return false;
}


void lexObject_init(lex_Object* object, char* source) {
    object->source = source;
    object->current = object->source;
    object->length = strlen(object->source);
    object->tokens = calloc(1000, sizeof(Token));
    object->token_size = 1000;
    object->token_used = 0;
}

void append_token(lex_Object* object, Token token) {
    if (object->token_used >= object->token_size) {
        object->token_size *= 2;
        object->tokens = realloc(object->tokens, object->token_size * sizeof(Token));
    }
    token.is_start = false;
    token.is_end = false;
    object->tokens[object->token_used++] = token;
}

Token create_token(char* value, TokenType type) {
    Token token;
    token.token = type;
    strcpy(token.value, value);
    return token;
}

void lex_comment(char* current_token, lex_Object** plexObject) {
    int index = 0;
    lex_Object* lexObject = *plexObject;
    while (NEXT_TOKEN(lexObject->current) != '\0' && !IS_END_OF_LINE(NEXT_TOKEN(lexObject->current))) { /* reached comment, loop through until end of line or end of code is reached. */
        current_token[index++] = *lexObject->current;
        lexObject->current++;
    }
    append_token(lexObject, create_token(current_token, COMMENT));
}

void lex_string(char* current_token, lex_Object** plexObject) {
    int index = 0;
    lex_Object* lexObject = *plexObject;
    char format_used = *lexObject->current;
    lexObject->current++;
    while (*lexObject->current != format_used) {
        /* TODO: Escape sequences should be translated here */
        current_token[index++] = *lexObject->current;
        lexObject->current++;
    }
    append_token(lexObject, create_token(current_token, SLITERAL));
}

void lex_num(char* current_token, int* index, lex_Object** plexObject) {
    lex_Object* lexObject = *plexObject;
    current_token[(*index)++] = *lexObject->current;
    if (IS_DECIMAL_POINT(*lexObject->current) && !IS_NUM(*(lexObject->current + 1))) { /* 1. -> 1.0 */
        current_token[(*index)++] = '0';
    }
}

void lex(lex_Object* lexObject) {
    char current_token[MAX_VALUE_SIZE];
    memset(current_token, 0, sizeof(current_token));
    int index = 0;
    bool is_collecting = false; /* identifier or keyword, we just don't know yet. */
    bool is_collecting_num = false;
    while (*lexObject->current != '\0') {
        if (*lexObject->current == '/' && NEXT_TOKEN(lexObject->current) == '/') {
            lex_comment(current_token, &lexObject);
            memset(current_token, 0, sizeof(current_token));
        } else if (*lexObject->current == '"' || *lexObject->current == '\'') {
            lex_string(current_token, &lexObject);
            memset(current_token, 0, sizeof(current_token));
        } else if (IS_TOKEN_SEPARATOR(*lexObject->current)) {
            if (is_collecting) {
                is_collecting = false;
                Token token = create_token(current_token, is_keyword(current_token) ? KEYWORD : IDENTIFIER);
                append_token(lexObject, token);
                index = 0;
                memset(current_token, 0, sizeof(current_token));
            } else if (is_collecting_num) {
                is_collecting_num = false;
                append_token(lexObject, create_token(current_token, ILITERAL));
                index = 0;
                memset(current_token, 0, sizeof(current_token));
            }
            if (is_operator(*lexObject->current) || is_punctuator(*lexObject->current)) {
                Token op;
                memset(op.value, 0, sizeof(op.value));
                op.token = is_operator(*lexObject->current) ? OPERATOR : PUNCTUATOR;
                op.value[0] = *lexObject->current;
                append_token(lexObject, op);
            }
        } else if (IS_IDENTIFIER_CHAR(*lexObject->current)) {
            if (!is_collecting)
                is_collecting = true;
            current_token[index++] = *lexObject->current;
        } else if (IS_NUM(*lexObject->current) || IS_DECIMAL_POINT(*lexObject->current)) {
            if (is_collecting) {
                current_token[index++] = *lexObject->current;
            } else {
                if (!is_collecting_num)
                    is_collecting_num = true;
                lex_num(current_token, &index, &lexObject);
            }
        }
        lexObject->current++;
    }
    lexObject->tokens[0].is_start = true;
    lexObject->tokens[lexObject->token_used - 1].is_end = true;
}

void analyze_syntax(lex_Object* lexObject) {
    //TODO
}

void lex_free(lex_Object* lexObject) {
    free(lexObject->tokens);
}
