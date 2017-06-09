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

			// get current line
			int GetLine(); 

			// get current position in line
			int GetPosition(); 

			Lexer(const Lexer& other) = delete; // disallow copy
			Lexer& operator =(const Lexer& other) = delete; // disallow copy

		private:
			void ParseNumber();
			void ParseIdOrString();
			void ParseString();
			void ParseWhitespace();
			int lineCounter, positionCounter, tokenStartPosition;
			Token token;
			std::string buffer;
			std::istringstream& input;
		};

	}
}
