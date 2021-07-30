#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "lex.h"

bool is_operator(char c) {
    switch (c) {
    case '.':
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


char* create_value(char* str) {
    char* m_str = malloc(strlen(str) + 1);
    strcpy(m_str, str);
    return m_str;
}

void lexObject_init(lex_Object* object, char* source) {
    object->source = source;
    object->length = strlen(object->source);
    object->tokens = (Token*)ss_malloc(250 * sizeof(Token));
    object->token_size = 250;
    object->token_used = 0;
}

void append_token(lex_Object* object, Token token) {
    if (object->token_used == object->token_size) {
        object->token_size *= 2;
        object->tokens = (Token*)realloc(object->tokens, object->token_size * sizeof(Token));
    }
    object->tokens[object->token_used++] = token;
}

Token create_token(char* value, TokenType type) {
    Token token;
    token.value = value;
    token.token = type;
    token.is_start = false;
    token.is_end = false;
    return token;
}

char* scan_text(lex_Object* object, char* current) {
    char source[IDENTIFIER_LIMIT];
    int used = 0;
    while (is_identifier_char(*current)) {
        source[used++] = *current;
        current++;
    }
    source[used++] = '\0';
    Token token;
    token.value = create_value(source);
    token.token = is_keyword(token.value) ? KEYWORD : IDENTIFIER;
    token.is_start = false;
    token.is_end = false;
    append_token(object, token);
    return current;
}

char* scan_operator(lex_Object* object, char* current) {
    char token_operator[3];
    int used = 0;
    switch (*current) {
    case '<':
    case '>':
    case '=':
        token_operator[used++] = *current++;
        if (*current == '=') {
            token_operator[used++] = '=';
            current++;
        }
        break;
    case '+':
    case '-':
    case '*':
    case '/':
    case ';':
    case '.':
    case ',':
        token_operator[used++] = *current++;
        break;
    default:
        ss_throw("Invalid operator %c\n", *current);
    }
    token_operator[used++] = '\0';
    append_token(object, create_token(create_value(token_operator), OPERATOR));
    return current;
}

char* scan_int(lex_Object* object, char* current) {
    char token_integer[22];
    int used = 0;
    while (is_num(*current) || is_decimal_point(*current)) {
        token_integer[used++] = *current++;
    }
    token_integer[used++] = '\0';
    append_token(object, create_token(create_value(token_integer), ILITERAL));
    return current;
}

char* scan_string(lex_Object* object, char* current, char string_end) {
    int size = 85;
    char* token_string = malloc(size);
    int used = 0;
    while (*current != string_end) {
        if (*current == '\\') {
            switch (*current++) {
            case 'n':
                token_string[used++] = '\n';
                break;
            case 't':
                token_string[used++] = '\t';
                break;
            case '\\':
                token_string[used++] = '\\';
                break;
            case '"':
                token_string[used++] = '"';
                break;
            case '\'':
                token_string[used++] = '\'';
                break;
            }
            current++;
        } else {
            token_string[used++] = *current++;
        }
        if ((used + 1) == size) {
            size *= 2;
            token_string = realloc(token_string, size);
        }
    }
    token_string[used++] = '\0';
    append_token(object, create_token(create_value(token_string), SLITERAL));
    free(token_string);
    return ++current;
}

char* skip_to_eol(char* current) {
    while (*current != '\n' && *current != '\0') {
        current++;
    }
    return current;
}


void lex(lex_Object* lexObject) {
    char* current = lexObject->source;
    while (*current != '\0') {
        if (*current == '/' && peek(current) == '/') {
            current = skip_to_eol(current);
        } else if (is_identifier_char(*current)) {
            current = scan_text(lexObject, current);
        } else if (is_operator(*current)) {
            current = scan_operator(lexObject, current);
        } else if (is_punctuator(*current)) {
            char punctuator[2];
            punctuator[0] = *current++;
            punctuator[1] = '\0';
            append_token(lexObject, create_token(create_value(punctuator), PUNCTUATOR));
        } else if (is_num(*current)) {
            current = scan_int(lexObject, current);
        } else if (*current == '"' || *current == '\'') {
            current++;
            current = scan_string(lexObject, current, *(current - 1));
        } else if (*current == '\n') {
            append_token(lexObject, create_token(create_value(""), NEWLINE));
            current++;
        } else {
            current++;
        }
    }
    if (lexObject->token_used != 0) {
        lexObject->tokens[0].is_start = true;
        lexObject->tokens[lexObject->token_used - 1].is_end = true;
    }
}

void lex_free(lex_Object* lexObject) {
    for (int64_t i = 0; i < lexObject->token_used; ++i) {
        free(lexObject->tokens[i].value);
    }
    free_and_null(lexObject->tokens);
}
