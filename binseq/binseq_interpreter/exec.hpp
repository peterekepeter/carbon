#pragma once
#include "parser.hpp"

namespace exec {

	enum COMMANDTYPE {
		END_STATEMENT,
		ASSIGN,
		CALL, //internal
		CALLBEGIN,
		CALLEND,
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		POSITIVE,
		NEGATIVE,
		BLOCK, //internal
		BLOCKBEGIN,
		BLOCKEND,
		COMP_LT,
		COMP_GT,
		COMP_LE,
		COMP_GE,
		COMP_EQ,
		COMP_NE,
		CONTROL,
		LOCAL,
		IF,
		IFELSE,
		LOOP0,
		LOOP1,
		LOOP2,
		LOOP3,
		RETURN0,
		RETURN1,
		BREAK,
		CONTINUE,
		FUNCTIONBEGIN,
		FUNCTIONEND
	};

	class ExecutorImp;

	class Executor {
	public:
		Executor();
		virtual ~Executor();
		void submitAtom(const yytokentype type, const char* text);
		void submitCommand(const COMMANDTYPE cmd);
		const char* getLastAtomText(); //for better syntax error message
		void SetShowPrompt(bool show);
		void Execute();
		void ClearStatement(); //parse error recovery for console mode
	private:
		ExecutorImp* imp;
	};

}
