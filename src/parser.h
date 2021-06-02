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
#define INDEX_OPEN "["
#define INDEX_CLOSE "]"
/* end of statement */
#define EOS ";"

#define MAX_IDENTIFIER_SIZE 31
#define MAX_ARGUMENTS 253

#define is_math_op(x) (x == '+' || x == '-' || x == '*' || x == '/')
#define is_literal(x) (x == ILITERAL || x == SLITERAL)

/* dereference macros */
#define get_variable(state) (*(ss_Variable*)state)
#define get_identifier(state) (*(ss_Identifier*)state)
#define get_operator(state) (*(ss_Operator*)state)
#define get_literal(state) (*(ss_Literal*)state)

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

typedef enum {
	l_INTEGER,
	l_STRING
} LiteralType;


typedef struct {
	StateType type;
	void* state;
} State;

typedef struct {
	State* states;
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
	ParseObject states;
} ss_Variable;

typedef struct {
	ss_Number value; // TODO: ss_Object
	LiteralType type;
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
void free_ParseObject(ParseObject* object);

#endif // SS_PARSER_H
