
#include "Token.h"
#include "Lexer.h"

Carbon::Compiler::Lexer::Lexer(std::istringstream & stream) 
	: input(stream)
	, lineCounter(1)
	, positionCounter(1)
	, tokenStartPosition(1)
	, token(Token::FileBegin)
{
	buffer.reserve(256); // initial buffer size
}

const char * Carbon::Compiler::Lexer::GetData()
{
	return this->buffer.c_str();
}

Carbon::Compiler::Token Carbon::Compiler::Lexer::GetToken()
{
	return this->token;
}

int Carbon::Compiler::Lexer::GetLine()
{
	return this->lineCounter;
}

int Carbon::Compiler::Lexer::GetPosition()
{
	return this->positionCounter;
}

void Carbon::Compiler::Lexer::ParseNumber()
{
	tokenStartPosition = positionCounter;
	buffer = "";
	bool floatingPoint = false;
	bool exponent = false;
	bool acceptSign = false;
	bool exponentSign = false;
	bool parsing = true;
	bool alphanumeric = false;
	while (parsing) {
		switch (input.peek()) {

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			acceptSign = false;
			buffer += input.get();
			break;

		case '.':
			if (!floatingPoint) {
				buffer += input.get();
				floatingPoint = true;
			}
			else {
				parsing = false; // can't have 2 dots, break
			}
			break;
		
		case 'e':
		case 'E':
			if (floatingPoint && !exponent) {
				acceptSign = true;
				exponent = true;
			}
			else {
				alphanumeric = true;
			}
			buffer += input.get();
			break;

		case '+':
		case '-':
			if (acceptSign) {
				// sign is part of fp exponent
				acceptSign = false;
				exponentSign = true;
				buffer += input.get();
			}
			else {
				// sign not part of float point number exponent, stop parsing
				parsing = false;
			}
			break;

			
		case 'a': case 'b': case 'c': case 'd':           case 'f': case 'g':
		case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
		case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D':           case 'F': case 'G':
		case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
		case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
		case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '_':
			// alphanumeric might be part of a special numeric type
			// like 0x1f 0b010011 and so on
			buffer += input.get();
			alphanumeric = true;
			break;


		default: // end of number
			parsing = false;
			break;
		}
	}
	positionCounter += buffer.size();

	// determine number type
	if (floatingPoint)
	{
		token = Token::Float;
	}
	else 
	{
		if (buffer[0] == '0') {
			auto second = buffer[1];
			switch (second) {
			case 'x': case 'X':
				token = Token::NumberHexadecimal;
				break;
			case 'b': case 'B':
				token = Token::NumberHexadecimal;
				break;
			default:
				if (alphanumeric) {
					token = Token::Number;
				}
				else {
					token = Token::NumberOctal;
				}
			}
		} else {
			token = Token::Number;
		}
	}
}

void Carbon::Compiler::Lexer::ParseIdOrString()
{

}

void Carbon::Compiler::Lexer::ParseString()
{
}


void Carbon::Compiler::Lexer::ParseWhitespace()
{
	bool parsing = true;
	while (parsing) {
		switch (input.peek()) {

		
		case '\n': // newline increase counter for that :)
			lineCounter++;
			positionCounter = 0;
		case ' ': // whitespace
		case '\t':
		case '\f':
		case '\r':
			input.get(); // consume whitespace chars
			positionCounter++;
			break;

		default: // end of whitespace, stop please
			parsing = false;
			break;
		}
	}
}

enum class LexerState
{

};

void Carbon::Compiler::Lexer::MoveNext()
{
	if (this->token == Token::FileEnd) {
		return;
	}

	bool parsing = true;
	while (parsing) {
		auto character = input.peek();
		switch (character) {

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': 
			ParseNumber();
			parsing = false;
			break;

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
		case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
		case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
		case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
		case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
		case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '_':
			ParseIdOrString();
			parsing = false;
			break;

		case '\f':
		case '\n':
		case '\t':
		case ' ':
			ParseWhitespace();
			break;

		// all chars should be handled
		case  std::char_traits<char>::eof():
			token = Token::FileEnd;
			parsing = false;
			break;

		default:
			token = Token::FileEnd;
			throw std::invalid_argument("Unexpected character.");
			break;
		}

	}
}


