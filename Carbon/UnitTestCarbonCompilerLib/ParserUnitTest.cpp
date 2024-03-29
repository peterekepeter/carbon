#include "stdafx.h"
#include "../CarbonCompilerLib/Lexer.h"
#include "../CarbonCompilerLib/Parser.h"

using namespace FRAMEWORK;

namespace UnitTestCarbonCompilerLib
{
	TEST_CLASS(ParserUnitTest)
	{
	public:
		using Type = Carbon::InstructionType;

		TEST_METHOD(ParserEmptyProgram)
		{
			Parsing(" ").ExpectEndOfFile();
		}

		TEST_METHOD(ParserEmptyProgramEmptyStatements)
		{
			Parsing(" ;;;; ").ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionAssignment)
		{
			Parsing("x=42")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("42")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}


		TEST_METHOD(ParserExpressionOpPriority)
		{
			Parsing("x=1+2*4-6/2")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("1")
				.Expect(Type::NUM).WithData("2")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::MULTIPLY)
				.Expect(Type::ADD)
				.Expect(Type::NUM).WithData("6")
				.Expect(Type::NUM).WithData("2")
				.Expect(Type::DIVIDE)
				.Expect(Type::SUBTRACT)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionSubExpression)
		{
			Parsing("x=3*(4+1)")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::NUM).WithData("1")
				.Expect(Type::ADD)
				.Expect(Type::MULTIPLY)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionSubSub)
		{
			Parsing("x=3*(4+2*(9-4)*7)")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::NUM).WithData("2")
				.Expect(Type::NUM).WithData("9")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::SUBTRACT)
				.Expect(Type::MULTIPLY)
				.Expect(Type::NUM).WithData("7")
				.Expect(Type::MULTIPLY)
				.Expect(Type::ADD)
				.Expect(Type::MULTIPLY)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionError)
		{
			// there is no ** operator, after parsing in x and 3 parsers should throw error
			auto& parser = Parsing("x=3**7)")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.ExpectException();
		}
		
		TEST_METHOD(ParserUnaryMinus)
		{
			Parsing("x=3*-7")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::NUM).WithData("7")
				.Expect(Type::NEGATIVE)
				.Expect(Type::MULTIPLY)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserUnaryPlus)
		{
			Parsing("x=3*+7")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::NUM).WithData("7")
				.Expect(Type::POSITIVE)
				.Expect(Type::MULTIPLY)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionObjectSimple)
		{
			Parsing("obj={}")
				.Expect(Type::ID).WithData("obj")
				.Expect(Type::OBJECTBEGIN)
				.Expect(Type::OBJECTEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionObject)
		{
			Parsing("v={x:3,y:4}")
				.Expect(Type::ID).WithData("v")
				.Expect(Type::OBJECTBEGIN)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::ID).WithData("y")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::OBJECTEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionArraySimple)
		{
			Parsing("v=[]")
				.Expect(Type::ID).WithData("v")
				.Expect(Type::ARRAYBEGIN)
				.Expect(Type::ARRAYEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserExpressionArray)
		{
			Parsing("v=[3,4]")
				.Expect(Type::ID).WithData("v")
				.Expect(Type::ARRAYBEGIN)
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::ARRAYEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserFunctionCallSimple)
		{
			Parsing("print()")
				.Expect(Type::ID).WithData("print")
				.Expect(Type::CALLBEGIN)
				.Expect(Type::CALLEND)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserFunctionCallParams)
		{
			Parsing("print(a,b,c)")
				.Expect(Type::ID).WithData("print")
				.Expect(Type::CALLBEGIN)
				.Expect(Type::ID).WithData("a")
				.Expect(Type::ID).WithData("b")
				.Expect(Type::ID).WithData("c")
				.Expect(Type::CALLEND)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserFunctionCallExpression)
		{
			Parsing("print(3+4)")
				.Expect(Type::ID).WithData("print")
				.Expect(Type::CALLBEGIN)
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::ADD)
				.Expect(Type::CALLEND)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserFunctionParameterless)
		{
			Parsing("hello = function () 42;;")
				.Expect(Type::ID).WithData("hello")
				.Expect(Type::FUNCTIONBEGIN)
				.Expect(Type::NUM).WithData("42")
				.Expect(Type::END_STATEMENT)
				.Expect(Type::FUNCTIONEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserFunction)
		{
			Parsing("double = function (x) x*x;;")
				.Expect(Type::ID).WithData("double")
				.Expect(Type::FUNCTIONBEGIN)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::MULTIPLY)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::FUNCTIONEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserEndStatementOptionalAfterFunctionClosingBraces)
		{
			Parsing("t=function(){return 3}t();")
				.Expect(Type::ID).WithData("t")
				.Expect(Type::FUNCTIONBEGIN)
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::RETURN1)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::BLOCKEND)
				.Expect(Type::FUNCTIONEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::ID).WithData("t")
				.Expect(Type::CALLBEGIN)
				.Expect(Type::CALLEND)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserFunctionMultiParam)
		{
			Parsing("math = function (x,y,z) x*y*z;;")
				.Expect(Type::ID).WithData("math")
				.Expect(Type::FUNCTIONBEGIN)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::ID).WithData("y")
				.Expect(Type::ID).WithData("z")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::ID).WithData("y")
				.Expect(Type::MULTIPLY)
				.Expect(Type::ID).WithData("z")
				.Expect(Type::MULTIPLY)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::FUNCTIONEND)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserLocal)
		{
			Parsing("local x")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::LOCAL)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserLocalAssign)
		{
			Parsing("local x=4")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::LOCAL)
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserMemberAssign)
		{
			Parsing("a.x=4;")
				.Expect(Type::ID).WithData("a")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::MEMBER)
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserBreakSimple)
		{
			Parsing("break;")
				.Expect(Type::BREAK)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserContinueSimple)
		{
			Parsing("continue;")
				.Expect(Type::CONTINUE)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserReturnSimple)
		{
			Parsing("return;")
				.Expect(Type::RETURN0)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserReturnExpression)
		{
			Parsing("return x*y;")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::ID).WithData("y")
				.Expect(Type::MULTIPLY)
				.Expect(Type::RETURN1)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserIfSimple)
		{
			Parsing("if (x<3) x=3;;")
				.Expect(Type::CONTROL)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::COMP_LT)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::IF)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserIfElseSimple)
		{
			Parsing("if (x<3) x=3; else x=4;;")
				.Expect(Type::CONTROL)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::COMP_LT)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::IFELSE)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserIfElseBlock)
		{
			Parsing("if (x<3) {x=3;} else {x=4;};")
				.Expect(Type::CONTROL)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::COMP_LT)
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("3")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::BLOCKEND)
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("4")
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::BLOCKEND)
				.Expect(Type::IFELSE)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserLoopSimple)
		{
			Parsing("loop {x=x-1;}")
				.Expect(Type::CONTROL)
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::ID).WithData("x")
				.Expect(Type::ID).WithData("x")
				.Expect(Type::NUM).WithData("1")
				.Expect(Type::SUBTRACT)
				.Expect(Type::ASSIGN)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::BLOCKEND)
				.Expect(Type::LOOP0)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserLoop1)
		{
			Parsing("loop(y<inner_y-12){}")
				.Expect(Type::CONTROL)
				// second expr
				.Expect(Type::ID).WithData("y")
				.Expect(Type::ID).WithData("inner_y")
				.Expect(Type::NUM).WithData("12")
				.Expect(Type::SUBTRACT)
				.Expect(Type::COMP_LT)
				// statement
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::BLOCKEND)
				.Expect(Type::LOOP1)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ParserLoop2)
		{
			Parsing("loop(y = 12, y<inner_y-12){}")
				.Expect(Type::CONTROL)
				// first expr
				.Expect(Type::ID).WithData("y")
				.Expect(Type::NUM).WithData("12")
				.Expect(Type::ASSIGN)
				// second expr
				.Expect(Type::ID).WithData("y")
				.Expect(Type::ID).WithData("inner_y")
				.Expect(Type::NUM).WithData("12")
				.Expect(Type::SUBTRACT)
				.Expect(Type::COMP_LT)
				// statement
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::BLOCKEND)
				.Expect(Type::LOOP2)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}
		TEST_METHOD(ParserLoop3)
		{
			Parsing("loop(y = 12, y<inner_y-12, y=y+1){}")
				.Expect(Type::CONTROL)
				// first expr
				.Expect(Type::ID).WithData("y")
				.Expect(Type::NUM).WithData("12")
				.Expect(Type::ASSIGN)
				// second expr
				.Expect(Type::ID).WithData("y")
				.Expect(Type::ID).WithData("inner_y")
				.Expect(Type::NUM).WithData("12")
				.Expect(Type::SUBTRACT)
				.Expect(Type::COMP_LT)
				// third expr
				.Expect(Type::ID).WithData("y")
				.Expect(Type::ID).WithData("y")
				.Expect(Type::NUM).WithData("1")
				.Expect(Type::ADD)
				.Expect(Type::ASSIGN)
				// statement
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::BLOCKEND)
				.Expect(Type::LOOP3)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ReturnWithoutParams)
		{
			Parsing("return")
				.Expect(Type::RETURN0)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(ReturnWithSingleParameter)
		{
			Parsing("return 42")
				.Expect(Type::NUM)
				.Expect(Type::RETURN1)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(FunctionOperator)
		{
			Parsing("x->x*x")
				.Expect(Type::ID)
				.Expect(Type::ID)
				.Expect(Type::ID)
				.Expect(Type::MULTIPLY)
				.Expect(Type::FUNCTION_OPERATOR)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_METHOD(FunctionOperatorWithFunctionBody) 
		{
			Parsing("x -> {return x}")
				.Expect(Type::ID)
				.Expect(Type::BLOCKBEGIN)
				.Expect(Type::ID)
				.Expect(Type::RETURN1)
				.Expect(Type::END_STATEMENT)
				.Expect(Type::BLOCKEND)
				.Expect(Type::FUNCTION_OPERATOR)
				.Expect(Type::END_STATEMENT)
				.ExpectEndOfFile();
		}

		TEST_ALL_METHOD()
		{
			RUN_TEST_METHOD(ParserEmptyProgram);
			RUN_TEST_METHOD(ParserEmptyProgramEmptyStatements);
			RUN_TEST_METHOD(ParserExpressionAssignment);
			RUN_TEST_METHOD(ParserExpressionOpPriority);
			RUN_TEST_METHOD(ParserExpressionSubExpression);
			RUN_TEST_METHOD(ParserExpressionSubSub);
			RUN_TEST_METHOD(ParserExpressionError);
			RUN_TEST_METHOD(ParserUnaryMinus);
			RUN_TEST_METHOD(ParserUnaryPlus);
			RUN_TEST_METHOD(ParserExpressionObjectSimple);
			RUN_TEST_METHOD(ParserExpressionObject);
			RUN_TEST_METHOD(ParserExpressionArraySimple);
			RUN_TEST_METHOD(ParserExpressionArray);
			RUN_TEST_METHOD(ParserFunctionCallSimple);
			RUN_TEST_METHOD(ParserFunctionCallParams);
			RUN_TEST_METHOD(ParserFunctionCallExpression);
			RUN_TEST_METHOD(ParserFunctionParameterless);
			RUN_TEST_METHOD(ParserFunction);
			RUN_TEST_METHOD(ParserEndStatementOptionalAfterFunctionClosingBraces);
			RUN_TEST_METHOD(ParserFunctionMultiParam);
			RUN_TEST_METHOD(ParserLocal);
			RUN_TEST_METHOD(ParserLocalAssign);
			RUN_TEST_METHOD(ParserMemberAssign);
			RUN_TEST_METHOD(ParserBreakSimple);
			RUN_TEST_METHOD(ParserContinueSimple);
			RUN_TEST_METHOD(ParserReturnSimple);
			RUN_TEST_METHOD(ParserReturnExpression);
			RUN_TEST_METHOD(ParserIfSimple);
			RUN_TEST_METHOD(ParserIfElseSimple);
			RUN_TEST_METHOD(ParserIfElseBlock);
			RUN_TEST_METHOD(ParserLoopSimple);
			RUN_TEST_METHOD(ParserLoop1);
			RUN_TEST_METHOD(ParserLoop2);
			RUN_TEST_METHOD(ParserLoop3);
			RUN_TEST_METHOD(ReturnWithoutParams);
			RUN_TEST_METHOD(ReturnWithSingleParameter);
			RUN_TEST_METHOD(FunctionOperator);
			RUN_TEST_METHOD(FunctionOperatorWithFunctionBody);
		}

	private:

		class Parsing
		{
			using Parser = Carbon::Parser;
			using Lexer = Carbon::Lexer;
			using StringStream = std::istringstream;

			StringStream stream;
			Lexer lexer;
			Parser parser;

		public:
			Parsing(const char* input) : stream(input), lexer(stream), parser(lexer) { }

			Parsing(const Parsing& other) = delete;
			Parsing(Parsing&& other) = delete;
			Parsing& operator = (const Parsing& other) = delete;
			Parsing& operator = (Parsing&& other) = delete;

			// continues parsing and check that it reaches and of file, without finding any more instructions
			Parsing& ExpectEndOfFile() {
				Assert::IsFalse(parser.MoveNext(), L"File should be ending.");
				return *this;
			}

			// continues parsing and check that next instruction is of given type
			Parsing& Expect(Type expectedType){
				if (parser.MoveNext() == false) {
					FailWithMessage(L"Expecting there to be more instructions.");
				}
				auto type = parser.ReadInstructionType();
				if (expectedType != type) {
					FailWithMessage(L"Expecting instruction to be the expected type.");
				}
				return *this;
			}

			// checks that the current instruction has data
			Parsing& WithData(const char* expectedData){
				auto parsedData = parser.ReadStringData();
				if (std::strcmp(parsedData, expectedData) != 0) {
					FailWithMessage(L"Parsed data was not as expected.", expectedData, parsedData);
				}
				return *this;
			}

			// continues parsing and expects the parser to throw an exception
			Parsing& ExpectException() {
				try {
					parser.MoveNext();
					FailWithMessage(L"Was expecting exception to be thrown, but none was.");
				}
				catch (Carbon::ParserException exception) {
					// ok
				}
				return *this;
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