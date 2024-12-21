#pragma once

#include <sstream>
#include "Token.h"
#include "ParserException.h"

namespace Carbon
{
	// transform sequence of characters to tokens
	class Lexer
	{
	public:
		// start parsing
		Lexer(std::istream& inputStream);

		// move to next token
		void MoveNext();

		// get token type
		Token GetToken();

		// get the indetified token data, like "Hello Wolrd"
		const char* GetData();

		// get a printable hardcoded string for the current token type
		const char* GetTokenStr();

		// get token start line
		int GetLine(); 

		// get token position in line
		int GetPosition(); 
			
		// return the ending position for token
		int GetPositionEnd();

		// return the ending line for token
		int GetLineEnd();

		Lexer(const Lexer& other) = delete; // disallow copy
		Lexer& operator =(const Lexer& other) = delete; // disallow copy

	private:
		void ParseNumber();
		void ParseIdOrString();
		void ParseString();
		void ParseWhitespace();
		bool ParseOperator();
		Token DetectKeyword(const std::string& data);
		void StartToken();
		int lineCounter, positionCounter, tokenStartPosition, tokenStartLine;
		Token token;
		std::string buffer;
		std::istream& input;
		// creates an exception to be thrown
		ParserException MakeError(const char* message);
	};

	
}
