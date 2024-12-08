

#include "stdafx.h"
#include "../CarbonCompilerLib/Lexer.h"
#include "../CarbonCompilerLib/Parser.h"
#include "../CarbonCompilerLib/Compiler.h"
#include "../CarbonCoreLib/Executor.h"

using namespace FRAMEWORK;

namespace UnitTestCarbonCompilerLib
{
	TEST_CLASS(ExecutionTest)
	{
	public:
		TEST_METHOD(EmptyReturnShouldNotFail)
		{
			Executing("y=function(x){return;};y();").HasNullResult();
		}

		TEST_METHOD(ReturningIntegerValue)
		{
			Executing("y=function(x){return 3;};y();").HasIntegerResult(3);
		}

		TEST_METHOD(ArithmeticOperatorOrderShouldBeRespected)
		{
			Executing("2+9*2-5/5").HasIntegerResult(19);
		}

		TEST_METHOD(LocalVariableInitializerHasAccessToGlobalFunctions)
		{
			Executing("t=function(){local x=object();return x;};t();").HasObjectResult();
		}

		TEST_METHOD(ComparingVoidWithIntegerIsFalse)
		{
			Executing("void==12").HasBitResult(false);
		}

		TEST_METHOD(ComparingIntegerWithVoidIsFalse)
		{
			Executing("12==void").HasBitResult(false);
		}

		TEST_METHOD(SemicolonOptionalBeforeClosingBracket) 
		{
			Executing("t=function(){return 3};t();").HasIntegerResult(3);
		}

		TEST_METHOD(SemicolonOptionalAfterClosingBracket)
		{
			Executing("t=function(){return 3}t();").HasIntegerResult(3);
		}

		TEST_METHOD(SemicolonOptionalAfterLastStatement)
		{
			Executing("t=function(){return 3;};t()").HasIntegerResult(3);
		}

		TEST_METHOD(ObjectNotationCreatesNewObjects)
		{
			Executing("f=_->({});a=f();b=f();a.x=4;b.x=7;a.x!=b.x;").HasBitResult(true);
		}

		TEST_METHOD(FunctionExpressionMostBasicForm)
		{
			Executing("f=x->x*x;f(3)").HasIntegerResult(9);
		}

		TEST_METHOD(FunctionExpressionMultiParams)
		{
			Executing("f=(x,y)->x*y;f(3,2)").HasIntegerResult(6);
		}

		TEST_METHOD(FunctionExpressionWithBody)
		{
			Executing("f=x->{return x*2};f(2)").HasIntegerResult(4);
		}

		TEST_METHOD(OperatorOrderSpecifiedThroughParanthesis)
		{
			Executing("(1+2)*3").HasIntegerResult(9);
		}

		TEST_METHOD(CanSetMemberOfObject)
		{
			Executing("a={};a.x=4;get(a,\"x\");").HasIntegerResult(4);
		}

		TEST_METHOD(DeepMemberAccess) 
		{
			Executing("a={};a.x={};a.x.y=2;a.x.y*2;").HasIntegerResult(4);
		}

		TEST_METHOD(ParseLargeInteger)
		{
			Executing("123456789123456789;").HasIntegerResult(123456789123456789);
		}

		TEST_METHOD(CastLargeInteger)
		{
			Executing("integer(\"123456789123456789\");").HasIntegerResult(123456789123456789);
		}

		TEST_ALL_METHOD()
		{
			RUN_TEST_METHOD(EmptyReturnShouldNotFail);
			RUN_TEST_METHOD(ReturningIntegerValue);
			RUN_TEST_METHOD(ArithmeticOperatorOrderShouldBeRespected);
			RUN_TEST_METHOD(LocalVariableInitializerHasAccessToGlobalFunctions);
			RUN_TEST_METHOD(ComparingVoidWithIntegerIsFalse);
			RUN_TEST_METHOD(ComparingIntegerWithVoidIsFalse);
			RUN_TEST_METHOD(SemicolonOptionalBeforeClosingBracket);
			RUN_TEST_METHOD(SemicolonOptionalAfterClosingBracket);
			RUN_TEST_METHOD(SemicolonOptionalAfterLastStatement);
			RUN_TEST_METHOD(ObjectNotationCreatesNewObjects);
			RUN_TEST_METHOD(FunctionExpressionMostBasicForm);
			RUN_TEST_METHOD(FunctionExpressionMultiParams);
			RUN_TEST_METHOD(FunctionExpressionWithBody);
			RUN_TEST_METHOD(OperatorOrderSpecifiedThroughParanthesis);
			RUN_TEST_METHOD(CanSetMemberOfObject);
			RUN_TEST_METHOD(DeepMemberAccess);
			RUN_TEST_METHOD(ParseLargeInteger);
			RUN_TEST_METHOD(CastLargeInteger);
		}


	private:

		class Executing
		{
			using Node = Carbon::Node;
			using Executor = Carbon::Executor;
			using NodeType = Carbon::NodeType;

			std::wstring message;
			Executor executor;
			bool failed;
			std::shared_ptr<Node> result;

		public:
			Executing(const char* str) {
				try
				{
					failed = true;
					Carbon::CompileString(str, executor);
					result = executor.Execute();
					failed = false;
				}
				catch (Carbon::ParserException exception)
				{
					std::wstringstream stream;
					stream << "Parser Error: " << exception.GetMessageWithLineAndPosition().c_str() << "\n";
					message = stream.str();
				}
				catch (Carbon::ExecutorExeption exception)
				{
					std::wstringstream stream;
					stream << "Runtime Error: " << exception.GetMessage() << "\n";
					message = stream.str();
				}
			}
			Executing& ShouldNotFail() {
				Assert::IsFalse(failed, message.c_str());
				return *this;
			}
			Executing& HaveResultType(NodeType type) {
				ShouldNotFail();
				if (type == result->GetNodeType()) {
					return *this;
				}
				Assert::Fail((std::wstringstream() 
					<< "Was expecting '" << Carbon::GetTypeText(type) 
					<< "' but found '" << Carbon::GetTypeText(result->GetNodeType())
					<< "' instead.").str().c_str());
				return *this;
			}
			Executing& HasNullResult() {
				HaveResultType(NodeType::None);
				return *this;
			}
			Executing& HasObjectResult() {
				HaveResultType(NodeType::DynamicObject);
				return *this;
			}
			Executing& HasIntegerResult(long long expectedValue) {
				HaveResultType(NodeType::Integer);
				auto integer = *reinterpret_cast<Carbon::NodeInteger*>(&*result);
				if (integer.Value != expectedValue) {
					Assert::Fail((std::wstringstream()
						<< "Was expecting '" << expectedValue
						<< "' but found '" << integer.Value
						<< "' instead.").str().c_str());
				}
				return *this;
			}
			Executing& HasBitResult(bool expectedValue) {
				HaveResultType(NodeType::Bit);
				auto bit = *reinterpret_cast<Carbon::NodeBit*>(&*result);
				if (bit.Value != expectedValue) {
					Assert::Fail((std::wstringstream()
						<< "Was expecting '" << expectedValue
						<< "' but found '" << bit.Value
						<< "' instead.").str().c_str());
				}
				return *this;
			}


		};

	};
}