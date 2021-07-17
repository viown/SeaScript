#include <string.h>
#include <stdlib.h>
#include "./debug.h"
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
    case ';':
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
    object->tokens = (Token*)ss_malloc(1000 * sizeof(Token));
    object->token_size = 1000;
    object->token_used = 0;
}

void append_token(lex_Object* object, Token token) {
    if (object->token_used == object->token_size) {
        object->token_size *= 2;
        object->tokens = (Token*)realloc(object->tokens, object->token_size * sizeof(Token));
    }
    token.is_start = false;
    token.is_end = false;
    object->tokens[object->token_used++] = token;
}

Token create_token(char* value, TokenType type) {
    Token token;
    token.token = type;
    if (value != NULL ) {
        strcpy(token.value, value);
    } else {
        strcpy(token.value, TOKEN_UNSET);
    }
    return token;
}

void lex_comment(char* current_token, lex_Object** plexObject) {
    int index = 0;
    lex_Object* lexObject = *plexObject;
    while (NEXT_TOKEN(lexObject->current) != '\0' && !IS_END_OF_LINE(NEXT_TOKEN(lexObject->current))) {
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
    bool is_collecting = false;
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
            if (IS_END_OF_LINE(*lexObject->current)) {
                append_token(lexObject, create_token(NULL, NEWLINE));
            } else if (is_operator(*lexObject->current) || is_punctuator(*lexObject->current)) {
                Token op;
                memset(op.value, 0, sizeof(op.value));
                op.token = is_operator(*lexObject->current) ? OPERATOR : PUNCTUATOR;
                op.value[0] = *lexObject->current;
                if (NEXT_TOKEN(lexObject->current) == '=') {
                    op.value[1] = '=';
                    lexObject->current++;
                }
                append_token(lexObject, op);
            }
        } else if (IS_IDENTIFIER_CHAR(*lexObject->current)) {
            if (!is_collecting)
                is_collecting = true;
            current_token[index++] = *lexObject->current;
        } else if (IS_NUM(*lexObject->current) || IS_DECIMAL_POINT(*lexObject->current)) {
            if (is_collecting) {
                if (*lexObject->current == '.') {
                    is_collecting = false;
                    Token token = create_token(current_token, is_keyword(current_token) ? KEYWORD : IDENTIFIER);
                    append_token(lexObject, token);
                    index = 0;
                    memset(current_token, 0, sizeof(current_token));
                    append_token(lexObject, create_token(".", OPERATOR));
                } else {
                    current_token[index++] = *lexObject->current;
                }
            } else {
                if (!is_collecting_num) {
                    is_collecting_num = true;
                }
                lex_num(current_token, &index, &lexObject);
            }
        }
        lexObject->current++;
    }
    lexObject->tokens[0].is_start = true;
    lexObject->tokens[lexObject->token_used - 1].is_end = true;
}

void lex_free(lex_Object* lexObject) {
    free_and_null(lexObject->tokens);
}
