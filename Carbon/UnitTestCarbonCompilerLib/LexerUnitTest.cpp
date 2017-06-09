#include "stdafx.h"
#include "CppUnitTest.h"
#include "../CarbonCompilerLib/Lexer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestCarbonCompilerLib
{		
	TEST_CLASS(LexerUnitTest)
	{
	public:
		using Lexer = Carbon::Compiler::Lexer;
		using Token = Carbon::Compiler::Token;
		
		TEST_METHOD(LexerWhitespace)
		{
			std::istringstream iss(" ");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

		TEST_METHOD(LexerNumber)
		{
			std::istringstream iss("13");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Number);
			Assert::AreEqual("13", lexer.GetData());
		}

		TEST_METHOD(LexerNumberFloat)
		{
			std::istringstream iss("3.14159");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Float);
			Assert::AreEqual("3.14159", lexer.GetData());
		}

		TEST_METHOD(LexerNumberFloatWithExponent)
		{
			std::istringstream iss("0.41e1000");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Float);
		}

		TEST_METHOD(LexerNumberFloatWithExponentPlus)
		{
			std::istringstream iss("0.41e+1000");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Float);
		}

		TEST_METHOD(LexerNumberFloatWithExponentMinus)
		{
			std::istringstream iss("0.41e-1000");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Float);
		}

		TEST_METHOD(LexerNumberHexadecimal)
		{
			std::istringstream iss("0x1f");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::NumberHexadecimal);
		}

		TEST_METHOD(LexerNumberOctal)
		{
			std::istringstream iss("0777");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::NumberOctal);
		}

		TEST_METHOD(LexerNumberBinary)
		{
			std::istringstream iss("0x1f");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::NumberHexadecimal);
		}

		TEST_METHOD(LexerNumberSequence)
		{
			std::istringstream iss("1 2 3");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Number);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Number);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Number);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}


	};
}