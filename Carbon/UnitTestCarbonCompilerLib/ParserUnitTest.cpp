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
		using InstructionType = Carbon::InstructionType;
		using Parser = Carbon::Parser;
		using Lexer = Carbon::Lexer;
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


		TEST_METHOD(ParserExpressionOpPriority)
		{
			ss input("x=1+2*4-6/2"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "1");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "2");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ADD);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "6");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "2");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::DIVIDE);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::SUBTRACT);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserExpressionSubExpression)
		{
			ss input("x=3*(4+1)"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "1");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ADD);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserExpressionSubSub)
		{
			ss input("x=3*(4+2*(9-4)*7)"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "2");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "9");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::SUBTRACT);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "7");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ADD);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserExpressionError)
		{
			// there is no ** operator, after parsing in x and 3 parsers should throw error
			ss input("x=3**7)"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::ExpectException<std::invalid_argument>([&]() { Assert::IsTrue(parser.MoveNext()); });
		}
		
		TEST_METHOD(ParserUnaryMinus)
		{
			ss input("x=3*-7"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "7");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NEGATIVE);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserUnaryPlus)
		{
			ss input("x=3*+7"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "7");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::POSITIVE);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}


		TEST_METHOD(ParserExpressionObjectSimple)
		{
			ss input("obj={}"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "obj");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::OBJECTBEGIN);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::OBJECTEND);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserExpressionObject)
		{
			ss input("v={x:3,y:4}"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "v");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::OBJECTBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::OBJECTEND);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserExpressionArraySimple)
		{
			ss input("v=[]"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "v");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ARRAYBEGIN);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ARRAYEND);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserExpressionArray)
		{
			ss input("v=[3,4]"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "v");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ARRAYBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ARRAYEND);
			Assert::IsTrue(parser.MoveNext());	Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserFunctionCallSimple)
		{
			ss input("print()"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "print");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CALLBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CALLEND);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
			
		}

		TEST_METHOD(ParserFunctionCallParams)
		{
			ss input("print(a,b,c)"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "print");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CALLBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "a");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "b");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "c");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CALLEND);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserFunctionCallExpression)
		{
			ss input("print(3+4)"); Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "print");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CALLBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM); Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM); Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ADD);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CALLEND);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());

		}




	};
}