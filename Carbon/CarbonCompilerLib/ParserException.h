#pragma once
#include <string>

namespace Carbon
{
	// This exception is thrown by both Parser and Lexer. There is not much 
	// sense making a separated exception class for Lexer. It can contain a
	// custom message but it will also contain the line posision where the
	// exception occured.
	class ParserException : public std::logic_error
	{
	public:
		// at which line did the error occure, -1 if unknown
		int Line;
		// position in line at which there was an error, -1 if unknown
		int LinePosition;

		std::string GetMessage() const;

		std::string GetMessageWithLineAndPosition() const;

		explicit ParserException(const std::string& _Message);

		explicit ParserException(const char* _Message);
	};

}