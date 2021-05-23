#ifndef SS_LEX_H
#define SS_LEX_H
#include <stdbool.h>

#define LEN(x) (sizeof(x) / sizeof(x[0]))

#define IS_WHITESPACE(x) (x == '\t' || x == ' ')
#define IS_END_OF_LINE(x) (x == '\n')
#define IS_CHAR(x) ((x >= 'A' && x <= 'Z') || (x >= 'a' && x <= 'z') || x == '_')
#define IS_NUM(x) (x >= '0' && x <= '9')


#define MAX_VALUE_SIZE 1000

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

enum TokenType {
	IDENTIFIER, // variable names, function names, etc
	KEYWORD, // e.g if, func, while, return, etc
	OPERATOR, // >=, <=, >, <, =, etc
	LITERAL, // booleans, strings, numbers, etc
	PUNCTUATOR, // (), {}, [], etc
	GLOBAL, // A reserved global, usually resolves into a certain value
	COMMENT, // like this
};
typedef enum TokenType TokenType;

struct Token {
	TokenType token;
	char value[MAX_VALUE_SIZE];
};
typedef struct Token Token;

struct lex_Object {
	char* source;
	char* current;
	size_t length;
	Token* tokens;
	size_t token_size;
	size_t token_used;
};
typedef struct lex_Object lex_Object;

bool is_operator(char c);
bool is_keyword(const char* c);
bool is_punctuator(char c);

void lexObject_init(lex_Object* object, char* source);
void lex(lex_Object* lexObject);
void analyze_syntax(lex_Object* lexObject);

void lex_free(lex_Object* lexObject);

#endif // SS_LEX_H
