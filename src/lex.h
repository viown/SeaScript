#ifndef SS_LEX_H
#define SS_LEX_H
#include <stdbool.h>

#define IS_WHITESPACE(x) (x == '\t' || x == ' ')
#define IS_END_OF_LINE(x) (x == '\n')
#define IS_CHAR(x) ((x >= 'A' && x <= 'Z') || (x >= 'a' && x <= 'z'))
#define IS_NUM(x) (x >= '0' && x <= '9')

static const char* const ss_keywords[] = {
	"if", "function", "while", "break", "return", "var"
};
static const int keyword_count = 6;

enum TokenType {
	IDENTIFIER, // variable names, function names, etc
	KEYWORD, // e.g if, func, while, return, etc
	OPERATOR, // >=, <=, >, <, =, etc
	LITERAL, // booleans, strings, numbers, etc
	PUNCTUATOR, // (), {}, [], etc
	COMMENT, // like this
};
typedef enum TokenType TokenType;

struct Token {
	TokenType token;
	char* value;
};
typedef struct Token Token;

struct lex_Object {
	char* source;
	size_t length;
	char* current;
};

bool is_operator(char c);
bool is_keyword(const char* c);
bool is_punctuator(char c);


void lex(Token* tokens, int length);

#endif // SS_LEX_H
