#ifndef SS_PARSER_H
#define SS_PARSER_H
#include <stddef.h>
#include "./lex.h"

/* symbols */
#define SCOPE_OPEN "{"
#define SCOPE_CLOSE "}"
#define FUNC_OPEN "("
#define FUNC_CLOSE ")"
#define ARRAY_OPEN "["
#define ARRAY_CLOSE "]"
#define INDEX_OPEN "["
#define INDEX_CLOSE "]"
#define ARG_SEPARATOR ","
#define ASSIGNMENT "="
/* comparison */
#define GREATER_THAN ">"
#define LESS_THAN "<"
#define GREATER_THAN_OR_EQUAL_TO ">="
#define LESS_THAN_OR_EQUAL_TO "<="
#define EQUAL_TO "=="
#define NOT_EQUAL_TO "!="
/* end of statement */
#define EOS ";"

#define MAX_IDENTIFIER_SIZE 31
#define MAX_ARGUMENTS 253

#define is_eq(x, y) (strcmp(x, y) == 0)
#define is_math_op(x) (x == '+' || x == '-' || x == '*' || x == '/')
#define is_comparison_op(x) (x == '>' || x == '<')
#define is_literal(x) (x == ILITERAL || x == SLITERAL)

/* dereference macros */
#define get_variable(state) (*(ss_Variable*)state)
#define get_identifier(state) (*(ss_Identifier*)state)
#define get_operator(state) (*(ss_Operator*)state)
#define get_literal(state) (*(ss_Literal*)state)
#define get_functioncall(state) (*(ss_FunctionCall*)state)
#define get_reassignment(state) (*(ss_Reassignment*)state)
#define get_index(state) (*(ss_IndexOperator*)state)
#define get_function(state) (*(ss_Function*)state)
#define get_return(state) (*(ss_ReturnStatement*)state)

#define load_literal(literal) (*(double*)literal.value)

typedef char Operator;

typedef enum {
    s_SCOPE,
    s_FUNCTION,
    s_FUNCTIONCALL,
    s_VARIABLE,
    s_REASSIGN,
    s_PRECEDENCE,
    s_LITERAL,
    s_OPERATOR,
    s_INDEX,
    s_IDENTIFIER,
    s_RETURN
} StateType;

typedef enum {
    MATH,
    COMPARISON,
} OperatorType;

typedef enum {
    l_INTEGER,
    l_STRING
} LiteralType;

typedef enum {
    v_GLOBAL,
    v_LOCAL
} VariableType;

typedef struct {
    void* state;
    StateType type;
} State;

typedef struct {
    char identifier[MAX_IDENTIFIER_SIZE];
} ss_Identifier;

typedef struct {
    ss_Identifier* arguments;
    int length;
} ArgumentCollection;

typedef struct {
    State* states;
    size_t length;
} ParseObject;

typedef ParseObject ss_Scope;
typedef ParseObject ss_Precedence;
typedef ParseObject ss_ReturnStatement;

typedef struct {
    char function_name[MAX_IDENTIFIER_SIZE];
    ArgumentCollection arguments;
    ParseObject scope;
    bool is_lamda;
} ss_Function;

typedef struct {
    char function_name[MAX_IDENTIFIER_SIZE];
    State* arguments;
    size_t arg_count;
} ss_FunctionCall;

typedef struct {
    char variable_name[MAX_IDENTIFIER_SIZE];
    ParseObject states;
    bool is_initialized;
} ss_Variable;

typedef struct {
    char variable_name[MAX_IDENTIFIER_SIZE];
    ParseObject states;
} ss_Reassignment;

typedef struct {
    void* value;
    LiteralType type;
} ss_Literal;

typedef struct {
    OperatorType type;
    void* op;
} ss_Operator;

typedef struct {
    char* name;
    ParseObject* parse_object;
} ss_IndexOperator;

bool variable_declared(Token* token, State* states, int length);
ParseObject parse(lex_Object object);
void free_ParseObject(ParseObject* object);

#endif // SS_PARSER_H
