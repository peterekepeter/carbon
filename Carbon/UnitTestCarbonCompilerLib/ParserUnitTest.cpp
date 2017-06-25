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




	};
}