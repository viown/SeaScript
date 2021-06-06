#include <stdio.h>
#include "./compiler.h" // for IS_INT
#include "./debug.h"

void visualize_tokens(lex_Object* object) {
	for (int i = 0; i < object->token_used; ++i) {
		printf("<SPECIFIER='%d', VALUE='%s', IS_START=%d, IS_END=%d>\n", object->tokens[i].token, object->tokens[i].value, object->tokens[i].is_start, object->tokens[i].is_end);
	}
}

/* visualizes the parse object into a tree-like structure */
void visualize_states(ParseObject* object) {
	for (int i = 0; i < object->length; ++i) {
		State current = object->states[i];
		if (current.type == s_VARIABLE) {
			ss_Variable var = get_variable(current.state);
			printf("%s = ", var.variable_name);
			for (int i = 0; i < var.states.length; ++i) {
				State st = var.states.states[i];
				if (st.type == s_IDENTIFIER) {
					printf("%s ", get_identifier(st.state).identifier);
				} else if (st.type == s_OPERATOR) {
					printf("%c ", *(Operator*)get_operator(st.state).op);
				} else if (st.type == s_LITERAL) {
					ss_Literal literal = get_literal(st.state);
					if (literal.type == l_INTEGER) {
						if (IS_INT(literal.value)) {
							int temp = (int)literal.value;
							printf("%d ", temp);
						} else {
							printf("%f ", literal.value);
						}
					}
				} else {
					printf("<unknown> ");
				}
			}
			printf("\n");
		}
	}
	printf("\n");
}

void visualize_token(Token* token) {
	printf("<SPECIFIER=%d, VALUE=%s>\n", token->token, token->value);
}
