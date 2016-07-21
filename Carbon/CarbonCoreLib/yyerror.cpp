#include "lexer.hpp"
#include "exec.hpp"
#include <cstdio>

void yyerror(const char* message) {
	std::fprintf(stderr, "Compiler: %s at line %d near '%s'\n", message, yylineno, yytext);
}
