#include "lexer.hpp"
#include "exec.hpp"
#include <cstdio>

void yyerror(const char* message) {
	std::fprintf(stderr, "%s at line %d near %s\n", message, yylineno, yytext);
}
