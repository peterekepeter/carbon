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

			// general parser states
			enum class State {
				Program,
				Statement,
				BlockOrObject,
				BlockOrObject2ndToken,
				BlockTemp, 
				Block,
				Object,
				ObjectTemp,
				Expression,
				ExpressionPopUnary,
			};

			// used for parsing expressions
			enum class Op
			{
				// state
				Term,
				Expression,
				Paranthesis,
				// unary ops
				UnaryPlus,
				UnaryMinus,
				// binary ops
				Add,
				Subtract,
				Multiply,
				Divide,
				Assign,
				// comparison
				Equals,
				NotEquals,
				Greater,
				GreaterOrEqual,
				Less,
				LessOrEqual
			};

			bool ParseProgram();
			bool ParseStatement();
			bool ParseBlockOrObject();
			bool ParseBlockOrObject2ndToken();
			bool ParseBlockTemp();
			bool ParseBlock();
			bool ParseObject();
			bool ParseObjectTemp();
			bool ParseExpression();
			bool ParseExpressionPopUnary();

			InstructionType OpToInstructionType(Op top) const;
			InstructionType TokenToAtom(Token token);
			Op TokenToBinaryOp(Token token);
			Op TokenToUnaryOp(Token token);

			// returns true if operator has one param
			bool OpIsUnary(Op op);

			// all op types generated by the lexer
			static constexpr int OpPrecedence(Op op) noexcept;

			// returns if op is right associative, otherwise false
			static constexpr bool OpIsRightAssociative(Op op) noexcept;

			InstructionType instruction;
			std::string instructionData;
			Lexer& lexer;
			std::stack<State> state;
			std::stack<Op> opStack;
			Token tempToken;
			std::string tempBuffer;
			bool expressionPrevAtom;
			bool expressionPrevOp;
			
			std::invalid_argument ParseError();
			std::invalid_argument ParseError(const char* message);

		};

		constexpr int Parser::OpPrecedence(Op op) noexcept
		{
			switch (op)
			{
			case Op::Assign:
				return 4;
			case Op::Equals:
			case Op::NotEquals:
				return 5;
			case Op::GreaterOrEqual:
			case Op::Greater:
			case Op::Less:
			case Op::LessOrEqual:
				return 6;
			case Op::Add:
			case Op::Subtract:
				return 7;
			case Op::Multiply:
			case Op::Divide:
				return 8;
			case Op::UnaryMinus:
			case Op::UnaryPlus:
				return 18;
			default:
				return 0;
			}
		}

		constexpr bool Parser::OpIsRightAssociative(Op op) noexcept
		{
			switch (op)
			{
			case Op::Assign:
				return true;
			default:
				return false;
			}
		}
	}
}