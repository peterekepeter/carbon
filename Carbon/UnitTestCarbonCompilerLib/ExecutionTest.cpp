

#include "stdafx.h"
#include "../CarbonCompilerLib/Lexer.h"
#include "../CarbonCompilerLib/Parser.h"
#include "../CarbonCompilerLib/Compiler.h"
#include "../CarbonCoreLib/Executor.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
				auto& integer = *reinterpret_cast<Carbon::NodeInteger*>(&*result);
				if (integer.Value != expectedValue) {
					Assert::Fail((std::wstringstream()
						<< "Was expecting '" << expectedValue
						<< "' but found '" << integer.Value
						<< "' instead.").str().c_str());
				}
				return *this;
			}


		};

	};
}