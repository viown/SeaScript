#include <string.h>
#include "./lex.h"

bool is_operator(char c) {
	switch (c) {
	case '>':
	case '<':
	case '=':
		return true;
	default:
		return false;
	}
}

bool is_keyword(const char* c) {
	for (size_t i = 0; i < keyword_count; ++i) {
		if (strcmp(ss_keywords[i], c) == 0) {
			return true;
		}
	}
	return false;
}

bool is_punctuator(char c) {
	switch (c) {
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
	case ';': // optional; but can be used to indicate the end of line.
		return true;
	default:
		return false;
	}
}
