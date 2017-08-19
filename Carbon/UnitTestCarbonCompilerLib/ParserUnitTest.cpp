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

		TEST_METHOD(ParserFunctionParameterless)
		{
			ss input("hello = function () 42;;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "hello");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::FUNCTIONBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM); Assert::AreEqual(parser.ReadStringData(), "42");
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::FUNCTIONEND);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserFunction)
		{
			ss input("double = function (x) x*x;;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "double");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::FUNCTIONBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::FUNCTIONEND);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserFunctionMultiParam)
		{
			ss input("math = function (x,y,z) x*y*z;;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "math");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::FUNCTIONBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "z");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID); Assert::AreEqual(parser.ReadStringData(), "z");
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::FUNCTIONEND);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext());  Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}


		TEST_METHOD(ParserLocal)
		{
			ss input("local x");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);		Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::LOCAL);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserBreakSimple)
		{
			ss input("break;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BREAK);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserContinueSimple)
		{
			ss input("continue;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTINUE);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserReturnSimple)
		{
			ss input("return;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::RETURN0);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserReturnExpression)
		{
			ss input("return x*y;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::MULTIPLY);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::RETURN1);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserIfSimple)
		{
			ss input("if (x<3) x=3;;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTROL);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::COMP_LT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::IF);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserIfElseSimple)
		{
			ss input("if (x<3) x=3; else x=4;;");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTROL);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::COMP_LT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::IFELSE);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserIfElseBlock)
		{
			ss input("if (x<3) {x=3;} else {x=4;};");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTROL);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::COMP_LT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "3");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKEND);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "4");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKEND);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::IFELSE);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserLoopSimple)
		{
			ss input("loop {x=x-1;}");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTROL);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "x");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "1");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::SUBTRACT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKEND);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::LOOP0);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserLoop1)
		{
			ss input("loop(y<inner_y-12){}");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTROL);
			// second expr
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "inner_y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "12");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::SUBTRACT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::COMP_LT);
			// statement
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKEND);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::LOOP1);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

		TEST_METHOD(ParserLoop2)
		{
			ss input("loop(y = 12, y<inner_y-12){}");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTROL);
			// first expr
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "12");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			// second expr
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "inner_y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "12");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::SUBTRACT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::COMP_LT);
			// statement
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKEND);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::LOOP2);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}
		TEST_METHOD(ParserLoop3)
		{
			ss input("loop(y = 12, y<inner_y-12, y=y+1){}");  Lexer lexer(input); Parser parser(lexer);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::CONTROL);
			// first expr
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "12");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			// second expr
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "inner_y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "12");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::SUBTRACT);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::COMP_LT);
			// third expr
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ID);	Assert::AreEqual(parser.ReadStringData(), "y");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::NUM);	Assert::AreEqual(parser.ReadStringData(), "1");
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ADD);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::ASSIGN);
			// statement
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKBEGIN);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::BLOCKEND);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::LOOP3);
			Assert::IsTrue(parser.MoveNext()); Assert::IsTrue(parser.ReadInstructionType() == InstructionType::END_STATEMENT);
			Assert::IsFalse(parser.MoveNext());
		}

	};
}