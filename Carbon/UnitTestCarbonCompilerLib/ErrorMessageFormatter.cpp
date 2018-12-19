#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestCarbonCompilerLib
{
	ErrorMessageFormatter::ErrorMessageFormatter(Lexer * lexer) : lexer(lexer) { }

	void ErrorMessageFormatter::FailWithMessage(const wchar_t * reason) {
		Assert::Fail(FormatMessage(reason).c_str());
	}
	void ErrorMessageFormatter::FailWithMessage(const wchar_t * reason, const char * expected, const char * actual) {
		Assert::Fail(FormatMessage(reason, expected, actual).c_str());
	}
	std::wstring ErrorMessageFormatter::FormatMessage(const wchar_t * message, const char * expected, const char * actual) {
		std::wstringstream builder;
		builder << "Expected \"" << expected << "\" but found \"" << actual << "\" instead!\n";
		FormatMessage(message, builder);
		return builder.str();
	}
	std::wstring ErrorMessageFormatter::FormatMessage(const wchar_t * message) {
		std::wstringstream builder;
		FormatMessage(message, builder);
		return builder.str();
	}
	void ErrorMessageFormatter::FormatMessage(const wchar_t * message, std::wstringstream & builder) {
		builder << message;
		if (lexer != nullptr) {
			builder << "\nAt line " << lexer->GetLine() << " pos " << lexer->GetPosition() << " near \"" << lexer->GetData() << "\"";
		}
	}
}