#ifndef SS_AST_H
#define SS_AST_H

typedef enum {
    AST_VALUE,
    AST_ADD,
    AST_MUL,
    AST_DIV,
    AST_SUB
} AbstractSyntaxTreeType;

struct AbstractSyntaxTree {
    int value;
    AbstractSyntaxTreeType type;
    AbstractSyntaxTree* right;
    AbstractSyntaxTree* left;
};

typedef struct AbstractSyntaxTree AbstractSyntaxTree;

#endif
