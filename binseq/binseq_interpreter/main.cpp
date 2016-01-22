#include "lexer.hpp"
#include "parser.hpp"
#include "exec.hpp"
#include <stdio.h> 
#include <stdlib.h>
#include <string>

exec::Executor ex;
int yyparse();
extern FILE* yyin;

std::string prepareString(const char* str) {
	std::string builder = "\"";
	char* p = (char*) str;
	while (*p) {
		if (*p == '\\') builder += "\\\\";
		else builder += *p;
		p++;
	}
	builder += '"';
	return builder;
}

void submitArguments(int argc, char** argv) {
	ex.submitAtom(yytokentype::ID, "arguments");
	ex.submitAtom(yytokentype::ID, "array");
	ex.submitCommand(exec::CALLBEGIN);
	char buffer[256];
	sprintf(buffer, "%d", argc);
	ex.submitAtom(yytokentype::NUM, buffer);
	ex.submitCommand(exec::CALLEND);
	ex.submitCommand(exec::ASSIGN);
	ex.submitCommand(exec::END_STATEMENT);

	for (int i = 0; i < argc; i++) {
		ex.submitAtom(yytokentype::ID, "set");
		ex.submitCommand(exec::CALLBEGIN);
		//1
		ex.submitAtom(yytokentype::ID, "arguments");
		//2
		char buffer[256];
		sprintf(buffer, "%d", i);
		ex.submitAtom(yytokentype::NUM, buffer);
		//3

		ex.submitAtom(yytokentype::STR, prepareString(argv[i]).c_str());

		ex.submitCommand(exec::CALLEND);
		ex.submitCommand(exec::END_STATEMENT);
	}
}

void submitEnv(char** env) {
	ex.submitAtom(yytokentype::ID, "environment");
	ex.submitAtom(yytokentype::ID, "object");
	ex.submitCommand(exec::CALLBEGIN);
	ex.submitCommand(exec::CALLEND);
	ex.submitCommand(exec::ASSIGN);
	ex.submitCommand(exec::END_STATEMENT);

	for (int i = 0; env[i] != nullptr; i++) {
		ex.submitAtom(yytokentype::ID, "set");
		ex.submitCommand(exec::CALLBEGIN);
		ex.submitAtom(yytokentype::ID, "environment");

		std::string e = env[i];
		auto pos = e.find('=');

		ex.submitAtom(yytokentype::STR, prepareString(e.substr(0, pos).c_str()).c_str());
		ex.submitAtom(yytokentype::STR, prepareString(e.substr(pos + 1, std::string::npos).c_str()).c_str());
		ex.submitCommand(exec::CALLEND);
		ex.submitCommand(exec::END_STATEMENT);
	}
}

int main(int argc, char** argv, char** env) {
	submitArguments(argc, argv);
	submitEnv(env);
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			yyin = fopen(argv[i], "r");
			if (yyin == 0) {
				fprintf(stderr, "cannot open file %s\n", argv[i]);
				exit(404);
			} else {
				yyparse();
				fclose(yyin);
				ex.Execute();
			}
		}
	} else
		while (true) {
			ex.SetShowPrompt(true);
			ex.ClearStatement();
			yyparse();
		}
}
