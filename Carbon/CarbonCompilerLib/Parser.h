#pragma once
#include "Lexer.h"
#include "../CarbonCommonLib/InstructionReader.h"
#include <stack>

namespace Carbon
{
	namespace Compiler
	{
		class Parser : InstructionReader
		{
		public:
			Parser(Lexer& lexer);

			virtual bool MoveNext();
			virtual InstructionType ReadInstructionType();
			virtual const char* ReadStringData();
		private:

			enum class State {
				Program,
				Statement
			};

			bool ParseProgram();
			bool ParseStatement();

			InstructionType instruction;
			Lexer& lexer;
			std::stack<State> state;
			
			std::invalid_argument ParseError();

		};
	}
}