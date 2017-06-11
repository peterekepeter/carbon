#pragma once

#include <sstream>
#include "Token.h"

namespace Carbon
{
	namespace Compiler
	{
		// transform sequence of characters to tokens
		class Lexer
		{
		public:
			// start parsing
			Lexer(std::istringstream& inputStream); 

			// move to next token
			void MoveNext();

			// get token type
			Token GetToken();

			// get the indetified token data, like "Hello Wolrd"
			const char* GetData();

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
			void ParseOperator();
			Token DetectKeyword(const std::string& data);
			void StartToken();
			int lineCounter, positionCounter, tokenStartPosition, tokenStartLine;
			Token token;
			std::string buffer;
			std::istringstream& input;
		};

	}
}
