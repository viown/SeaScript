#ifndef SS_LEX_H
#define SS_LEX_H
#include <stdint.h>
#include <stdbool.h>

#define LEN(x) (sizeof(x) / sizeof(x[0]))

#define is_whitespace(x) (x == '\t' || x == ' ')
#define is_end_of_line(x) (x == '\n')
#define is_char(x) ((x >= 'A' && x <= 'Z') || (x >= 'a' && x <= 'z'))
#define is_num(x) (x >= '0' && x <= '9')
#define is_identifier_char(x) (is_char(x) || x == '_')
#define is_decimal_point(x) (x == '.')

#define NEXT_TOKEN(x) (*(x+1))
#define PREVIOUS_TOKEN(x) (*(x-1))

#define peek(x) NEXT_TOKEN(x)

#define IS_START_TOKEN(token) (token->is_start)
#define IS_END_TOKEN(token) (token->is_end)

#define IDENTIFIER_LIMIT 85

static const char* const ss_keywords[] = {
    "if", "function", "while", "break",
    "return", "global", "local", "load",
    "for", "in", "not", "and", "or", "else",
    "elseif"
};
static const int keyword_count = LEN(ss_keywords);


typedef enum {
    IDENTIFIER, /* variable names, function names, etc */
    KEYWORD, /* e.g if, func, while, return, etc */
    OPERATOR, /* >=, <=, >, <, =, etc */
    ILITERAL, /* integer or float */
    SLITERAL, /* string literal */
    PUNCTUATOR, /* (), {}, [], etc */
    GLOBAL, /* A reserved global, usually resolves into a certain value */
    COMMENT, /* like this */
    TNEWLINE, /* end of line */
} TokenType;

typedef struct {
    TokenType token;
    char* value;

    /* bools to indicate start/end of token stream */
    bool is_start;
    bool is_end;
} Token;

typedef struct {
    char* source;
    int64_t length;
    Token* tokens;
    int64_t token_size;
    int64_t token_used;
} lex_Object;

void lexObject_init(lex_Object* object, char* source);
void lex(lex_Object* lexObject);
void lex_free(lex_Object* lexObject);

#endif // SS_LEX_H
