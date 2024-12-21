#include "stdafx.h"
#include "../CarbonCompilerLib/Lexer.h"

using namespace FRAMEWORK;

namespace UnitTestCarbonCompilerLib
{		
	TEST_CLASS(LexerUnitTest)
	{
	public:
		using Token = Carbon::Token;
		
		TEST_METHOD(LexerWhitespace)
		{
			Lexing(" ")
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerLineComment)
		{
			Lexing("//a=0")
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerNumber)
		{
			Lexing("13")
				.Expect(Token::Number).WithData("13");
		}

		TEST_METHOD(LexerNumberFloat)
		{
			Lexing("3.14159")
				.Expect(Token::Float).WithData("3.14159");
		}

		TEST_METHOD(LexerNumberFloatWithExponent)
		{
			Lexing("0.41e1000")
				.Expect(Token::Float);
		}

		TEST_METHOD(LexerNumberFloatWithExponentPlus)
		{
			Lexing("0.41e+1000")
				.Expect(Token::Float);
		}

		TEST_METHOD(LexerNumberFloatWithExponentMinus)
		{
			Lexing("0.41e-1000")
				.Expect(Token::Float);
		}

		TEST_METHOD(LexerNumberHexadecimal)
		{
			Lexing("0x1f")
				.Expect(Token::NumberHexadecimal);
		}

		TEST_METHOD(LexerNumberOctal)
		{
			Lexing("0777")
				.Expect(Token::NumberOctal);
		}

		TEST_METHOD(LexerNumberBinary)
		{
			Lexing("0x1f")
				.Expect(Token::NumberHexadecimal);
		}

		TEST_METHOD(LexerNumberSequence)
		{
			Lexing("1 2 3")
				.Expect(Token::Number)
				.Expect(Token::Number)
				.Expect(Token::Number)
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerIdentifiers)
		{
			Lexing("myVariable _sprintf MyClass variable44")
				.Expect(Token::Id).WithData("myVariable")
				.Expect(Token::Id).WithData("_sprintf")
				.Expect(Token::Id).WithData("MyClass")
				.Expect(Token::Id).WithData("variable44")
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerTokenPositions)
		{
			Lexing lexing("1 2 3\n4 5 6\n7 8 9");
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					lexing.Expect(Token::Number);
					Assert::AreEqual(j * 2 + 1, lexing.GetPosition());
					Assert::AreEqual(i + 1, lexing.GetLine());
				}
			}
			lexing.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerStringBasic)
		{
			Lexing("\"Hello World!\"")
				.Expect(Token::String).WithData("\"Hello World!\"")
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerStringWithEscape)
		{
			Lexing("\"Someone said \\\"Hello World!\\\"\"")
				.Expect(Token::String).WithData("\"Someone said \\\"Hello World!\\\"\"")
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerStringsMultiple)
		{
			Lexing("\"a\" \"b\" \"c\"")
				.Expect(Token::String).WithData("\"a\"")
				.Expect(Token::String).WithData("\"b\"")
				.Expect(Token::String).WithData("\"c\"")
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerStringTypes)
		{
			Lexing("b\"011011\" L\"b\" custom\"c\"")
				.Expect(Token::StringBinary).WithData("b\"011011\"")
				.Expect(Token::StringUnicode).WithData("L\"b\"")
				.Expect(Token::String).WithData("custom\"c\"")
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerKeywords)
		{
			Lexing("function local if else break continue return")
				.Expect(Token::Function)
				.Expect(Token::Local)
				.Expect(Token::If)
				.Expect(Token::Else)
				.Expect(Token::Break)
				.Expect(Token::Continue)
				.Expect(Token::Return)
				.Expect(Token::FileEnd);
		}


		TEST_METHOD(LexerOperators)
		{
			Lexing("= -> + - * / == != <= >= < > !")
				.Expect(Token::Assign)
				.Expect(Token::FunctionOperator)
				.Expect(Token::Plus)
				.Expect(Token::Minus)
				.Expect(Token::Multiply)
				.Expect(Token::Divide)
				.Expect(Token::Equals)
				.Expect(Token::NotEquals)
				.Expect(Token::LessOrEqual)
				.Expect(Token::GreaterOrEqual)
				.Expect(Token::Less)
				.Expect(Token::Greater)
				.Expect(Token::Not)
				.Expect(Token::FileEnd);
		}


		TEST_METHOD(LexerSyntaxSymbols)
		{
			Lexing("(){;}[]")
				.Expect(Token::ParanthesisOpen)
				.Expect(Token::ParanthesisClose)
				.Expect(Token::BracesOpen)
				.Expect(Token::EndStatement)
				.Expect(Token::BracesClose)
				.Expect(Token::BracketOpen)
				.Expect(Token::BracketClose)
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerSyntaxTestAssignment)
		{
			Lexing("x=4*y+13;")
				.Expect(Token::Id).WithData("x")
				.Expect(Token::Assign)
				.Expect(Token::Number).WithData("4")
				.Expect(Token::Multiply)
				.Expect(Token::Id).WithData("y")
				.Expect(Token::Plus)
				.Expect(Token::Number).WithData("13")
				.Expect(Token::EndStatement)
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerSyntaxTestFunction)
		{
			Lexing("function double(x) { return x*2; }")
				.Expect(Token::Function)
				.Expect(Token::Id).WithData("double")
				.Expect(Token::ParanthesisOpen)
				.Expect(Token::Id).WithData("x")
				.Expect(Token::ParanthesisClose)
				.Expect(Token::BracesOpen)
				.Expect(Token::Return)
				.Expect(Token::Id).WithData("x")
				.Expect(Token::Multiply)
				.Expect(Token::Number).WithData("2")
				.Expect(Token::EndStatement)
				.Expect(Token::BracesClose)
				.Expect(Token::FileEnd);
		}

		TEST_METHOD(LexerFileBeginEnd)
		{
			Lexing("")
				.CheckCurrentToken(Token::FileBegin)
				.Expect(Token::FileEnd);
		}

		void TestAll() {
			RUN_TEST_METHOD(LexerWhitespace);
			RUN_TEST_METHOD(LexerLineComment);
			RUN_TEST_METHOD(LexerNumber);
			RUN_TEST_METHOD(LexerNumberFloat);
			RUN_TEST_METHOD(LexerNumberFloatWithExponent);
			RUN_TEST_METHOD(LexerNumberFloatWithExponentPlus);
			RUN_TEST_METHOD(LexerNumberFloatWithExponentMinus);
			RUN_TEST_METHOD(LexerNumberHexadecimal);
			RUN_TEST_METHOD(LexerNumberOctal);
			RUN_TEST_METHOD(LexerNumberBinary);
			RUN_TEST_METHOD(LexerNumberSequence);
			RUN_TEST_METHOD(LexerIdentifiers);
			RUN_TEST_METHOD(LexerTokenPositions);
			RUN_TEST_METHOD(LexerStringBasic);
			RUN_TEST_METHOD(LexerStringWithEscape);
			RUN_TEST_METHOD(LexerStringsMultiple);
			RUN_TEST_METHOD(LexerStringTypes);
			RUN_TEST_METHOD(LexerKeywords);
			RUN_TEST_METHOD(LexerOperators);
			RUN_TEST_METHOD(LexerSyntaxSymbols);
			RUN_TEST_METHOD(LexerSyntaxTestAssignment);
			RUN_TEST_METHOD(LexerSyntaxTestFunction);
			RUN_TEST_METHOD(LexerFileBeginEnd);
		}

	private:
	
		class Lexing
		{
			using Lexer = Carbon::Lexer;
			using StringStream = std::istringstream;

			StringStream stream;
			Lexer lexer;

		public:
			Lexing(const char* input) : stream(input), lexer(stream) { }
			Lexing(const Lexing& other) = delete;
			Lexing(Lexing&& other) = delete;
			Lexing& operator = (const Lexing& other) = delete;
			Lexing& operator = (Lexing&& other) = delete;

			Lexing& Expect(Token token) {
				lexer.MoveNext();
				return CheckCurrentToken(token);
			}

			Lexing& CheckCurrentToken(Token token) {
				if (lexer.GetToken() != token) {
					FailWithMessage(L"Was expecting a different token.");
				}
				return *this;
			}

			Lexing& WithData(const char* expected) {
				auto data = lexer.GetData();
				if (std::strcmp(expected, data) != 0) {
					FailWithMessage(L"Token data does not match.", expected, lexer.GetData());
				}
				return *this;
			}

			int GetPosition() {
				return lexer.GetPosition();
			}

			int GetLine() {
				return lexer.GetLine();
			}

		private:

			void FailWithMessage(const wchar_t* reason) {
				ErrorMessageFormatter(&lexer).FailWithMessage(reason);
			}

			void FailWithMessage(const wchar_t* reason, const char* expected, const char* actual) {
				ErrorMessageFormatter(&lexer).FailWithMessage(reason, expected, actual);
			}

		};

	};
}