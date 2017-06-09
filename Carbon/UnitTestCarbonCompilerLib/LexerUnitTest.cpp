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

		TEST_METHOD(LexerIdentifiers)
		{
			std::istringstream iss("myVariable _sprintf MyClass variable44");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Id);
			Assert::AreEqual("myVariable", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Id);
			Assert::AreEqual("_sprintf", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Id);
			Assert::AreEqual("MyClass", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Id);
			Assert::AreEqual("variable44", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

		TEST_METHOD(LexerTokenPositions)
		{
			std::istringstream iss("1 2 3\n4 5 6\n7 8 9");
			Lexer lexer(iss);
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					lexer.MoveNext();
					Assert::IsTrue(lexer.GetToken() == Token::Number);
					Assert::AreEqual(j * 2 + 1, lexer.GetPosition());
					Assert::AreEqual(i + 1, lexer.GetLine());
				}
			}
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

		TEST_METHOD(LexerStringBasic)
		{
			std::istringstream iss("\"Hello World!\"");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::String);
			Assert::AreEqual("\"Hello World!\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

		TEST_METHOD(LexerStringWithEscape)
		{
			std::istringstream iss("\"Someone said \\\"Hello World!\\\"\"");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::String);
			Assert::AreEqual("\"Someone said \\\"Hello World!\\\"\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

		TEST_METHOD(LexerStringsMultiple)
		{
			std::istringstream iss("\"a\" \"b\" \"c\"");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::String);
			Assert::AreEqual("\"a\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::String);
			Assert::AreEqual("\"b\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::String);
			Assert::AreEqual("\"c\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

		TEST_METHOD(LexerStringTypes)
		{
			std::istringstream iss("b\"011011\" L\"b\" custom\"c\"");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::StringBinary);
			Assert::AreEqual("b\"011011\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::StringUnicode);
			Assert::AreEqual("L\"b\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::String);
			Assert::AreEqual("custom\"c\"", lexer.GetData());
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

		TEST_METHOD(LexerKeywords)
		{
			std::istringstream iss("function local if else break continue return");
			Lexer lexer(iss);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Function);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Local);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::If);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Else);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Break);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Continue);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::Return);
			lexer.MoveNext();
			Assert::IsTrue(lexer.GetToken() == Token::FileEnd);
		}

	};
}