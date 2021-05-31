#ifndef SS_PARSER_H
#define SS_PARSER_H
#include "./lex.h"
#include "./sstypes.h"

#define SCOPE_OPEN "{"
#define SCOPE_CLOSE "}"
#define FUNC_OPEN "("
#define FUNC_CLOSE ")"
#define ARRAY_OPEN "["
#define ARRAY_CLOSE "]"
/* end of statement */
#define EOS ";"

#define MAX_IDENTIFIER_SIZE 31
#define MAX_ARGUMENTS 253

#define IS_MATH_OP(x) (x == '+' || x == '-' || x == '*' || x == '/')

typedef int64_t IndexValue;
typedef char Operator;

typedef enum {
	s_SCOPE,
	s_FUNCTION,
	s_FUNCTIONCALL,
	s_VARIABLE,
	s_LITERAL,
	s_OPERATOR,
	s_IDENTIFIER,
} StateType;

typedef enum {
	MATH,
	COMPARISON,
	INDEX
} OperatorType;


typedef struct {
	StateType type;
	void* state;
} State;

typedef struct {
	State* begin;
	size_t length;
} ParseObject;

typedef struct {
	State* states;
} Scope;

typedef struct {
	char function_name[MAX_IDENTIFIER_SIZE];
	Scope* scope;
} ss_Function;

typedef struct {
	char function_name[MAX_IDENTIFIER_SIZE];
	ss_Number arguments[MAX_ARGUMENTS]; // TODO: ss_Object
} ss_FunctionCall;

typedef struct {
	char variable_name[MAX_IDENTIFIER_SIZE];
	State* states;
} ss_Variable;

typedef struct {
	ss_Number value; // TODO: ss_Object
} ss_Literal;

typedef struct {
	OperatorType type;
	void* op;
} ss_Operator;

typedef struct {
	char identifier[MAX_IDENTIFIER_SIZE];
} ss_Identifier;

ss_Object to_object(Token token);

ParseObject parse(lex_Object object);

#endif // SS_PARSER_H
