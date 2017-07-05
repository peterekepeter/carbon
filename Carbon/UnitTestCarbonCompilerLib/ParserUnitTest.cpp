#include "stdafx.h"
#include "CppUnitTest.h"
#include "../CarbonCompilerLib/Lexer.h"
#include "../CarbonCompilerLib/Parser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestCarbonCompilerLib
{
	TEST_CLASS(ParserUnitTest)
	{
	public:
		using Parser = Carbon::Compiler::Parser;
		using Lexer = Carbon::Compiler::Lexer;
		using ss = std::istringstream;

		TEST_METHOD(ParserEmptyProgram)
		{
			ss input(" "); Lexer lexer(input); Parser parser(lexer);
			auto result = parser.MoveNext();
			Assert::IsFalse(result);
		}

		TEST_METHOD(ParserEmptyProgramEmptyStatements)
		{
			ss input(" ;;;; "); Lexer lexer(input); Parser parser(lexer);
			auto result = parser.MoveNext();
			Assert::IsFalse(result);
		}

		TEST_METHOD(ParserExpressionAssignment)
		{
			ss input("x=42"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext());
			Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);
			Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());
			Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);
			Assert::AreEqual(parser.ReadStringData(), "42");
			Assert::IsTrue(parser.MoveNext());
			Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());
			Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}




	};
}