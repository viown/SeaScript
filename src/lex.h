#ifndef SS_LEX_H
#define SS_LEX_H
#include <stdint.h>
#include <stdbool.h>

#define LEN(x) (sizeof(x) / sizeof(x[0]))

#define IS_WHITESPACE(x) (x == '\t' || x == ' ')
#define IS_END_OF_LINE(x) (x == '\n')
#define IS_CHAR(x) ((x >= 'A' && x <= 'Z') || (x >= 'a' && x <= 'z'))
#define IS_NUM(x) (x >= '0' && x <= '9')
#define IS_IDENTIFIER_CHAR(x) (IS_CHAR(x) || x == '_')
#define IS_DECIMAL_POINT(x) (x == '.')

#define IS_TOKEN_SEPARATOR(character) (IS_WHITESPACE(character) || IS_END_OF_LINE(character) || is_operator(character) || is_punctuator(character))

#define NEXT_TOKEN(x) (*(x+1))
#define PREVIOUS_TOKEN(x) (*(x-1))

#define IS_START_TOKEN(token) (token->is_start)
#define IS_END_TOKEN(token) (token->is_end)

#define MAX_VALUE_SIZE 1000
#define TOKEN_UNSET ""

static const char* const ss_keywords[] = {
    "if", "function", "while", "break",
    "return", "global", "local", "load",
    "for", "in", "not", "and", "or"
};
static const int keyword_count = LEN(ss_keywords);

static const char* const ss_globals[] = {
    "true", // 1
    "false", // 0
    "inf", // (value > inf) will always be false
    "_debug" // true if running in debug env
};
static const int global_count = LEN(ss_globals);

typedef enum {
    IDENTIFIER, /* variable names, function names, etc */
    KEYWORD, /* e.g if, func, while, return, etc */
    OPERATOR, /* >=, <=, >, <, =, etc */
    ILITERAL, /* integer or float */
    SLITERAL, /* string literal */
    PUNCTUATOR, /* (), {}, [], etc */
    GLOBAL, /* A reserved global, usually resolves into a certain value */
    COMMENT, /* like this */
    NEWLINE, /* end of line */
} TokenType;

typedef struct {
    TokenType token;
    char value[MAX_VALUE_SIZE];

    /* bools to indicate start/end of token stream */
    bool is_start;
    bool is_end;
} Token;

typedef struct {
    char* source;
    char* current;
    int64_t length;
    Token* tokens;
    int64_t token_size;
    int64_t token_used;
} lex_Object;

bool is_operator(char c);
bool is_keyword(const char* c);
bool is_punctuator(char c);

void lexObject_init(lex_Object* object, char* source);
void lex(lex_Object* lexObject);

void lex_free(lex_Object* lexObject);

#endif // SS_LEX_H
