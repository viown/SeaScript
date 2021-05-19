#ifndef SS_LEX_H
#define SS_LEX_H

enum TokenType {
	IDENTIFIER, // variable names, function names, etc
	KEYWORD, // e.g if, func, while, return, etc
	OPERATOR, // >=, <=, >, <, =, etc
	LITERAL, // booleans, strings, numbers, etc
	COMMENT, // like this
};
typedef enum TokenType TokenType;


struct Token {
	TokenType token;
	char* value;
};

#endif // SS_LEX_H
