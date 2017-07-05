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
				Statement,
				BlockOrObject,
				BlockOrObject2ndToken,
				BlockTemp, 
				Block,
				Object,
				ObjectTemp,
				Expression
			};

			bool ParseProgram();
			bool ParseStatement();
			bool ParseBlockOrObject();
			bool ParseBlockOrObject2ndToken();
			bool ParseBlockTemp();
			bool ParseBlock();
			bool ParseObject();
			bool ParseObjectTemp();
			InstructionType TokenToInfixOperator(Token top) const;
			bool ParseExpression();

			InstructionType TokenToAtom(Token token);

			InstructionType instruction;
			std::string instructionData;
			Lexer& lexer;
			std::stack<State> state;
			std::stack<Token> opStack;
			Token tempToken;
			std::string tempBuffer;
			
			std::invalid_argument ParseError();
			std::invalid_argument ParseError(const char* message);

		};
	}
}