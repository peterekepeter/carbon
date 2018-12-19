#pragma once
#include "../CarbonCompilerLib/Lexer.h"

namespace UnitTestCarbonCompilerLib
{
	class ErrorMessageFormatter
	{
		using Lexer = Carbon::Lexer;

		Lexer* lexer;

	public:

		ErrorMessageFormatter(Lexer* lexer);

		void FailWithMessage(const wchar_t* reason);

		void FailWithMessage(const wchar_t* reason, const char* expected, const char* actual);

		std::wstring FormatMessage(const wchar_t* message, const char* expected, const char* actual);

		std::wstring FormatMessage(const wchar_t* message);

		void FormatMessage(const wchar_t* message, std::wstringstream& builder);

	};

}