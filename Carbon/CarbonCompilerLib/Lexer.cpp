
#include "Token.h"
#include "Lexer.h"

Carbon::Compiler::Lexer::Lexer(std::istringstream & stream) 
	: input(stream)
	, lineCounter(1)
	, positionCounter(1)
	, tokenStartPosition(1)
	, tokenStartLine(1)
	, token(Token::FileBegin)
{
	buffer.reserve(256); // initial buffer size
}

const char* Carbon::Compiler::Lexer::GetData()
{
	return this->buffer.c_str();
}

Carbon::Compiler::Token Carbon::Compiler::Lexer::GetToken()
{
	return this->token;
}

int Carbon::Compiler::Lexer::GetLine()
{
	return this->tokenStartLine;
}

int Carbon::Compiler::Lexer::GetPosition()
{
	return this->tokenStartPosition;
}

// parse in a number, consuming character from input
void Carbon::Compiler::Lexer::ParseNumber()
{
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

// parse in a string or id, consuming character from input
void Carbon::Compiler::Lexer::ParseIdOrString()
{
	bool string = false;
	bool parsing = true;
	while (parsing) {
		switch (input.peek()) {

		case '"':
			// ok seems like we have a string with prefix instead
			positionCounter += buffer.size();
			ParseString();
			parsing = false;
			string = true;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			buffer += input.get();
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
			buffer += input.get();
			break;

		default: 
			parsing = false;
			break;
		}
	}

	// type detect
	if (!string) {
		positionCounter += buffer.size();
		token = DetectKeyword(buffer);
	}

}

// parse in a string, consuming character from input
void Carbon::Compiler::Lexer::ParseString()
{
	bool escape = true;
	bool parsing = true;
	while (parsing) {
		switch (input.peek()) {

		case '"':
			buffer += input.get();
			positionCounter++;
			if (!escape) {
				// string just ended
				parsing = false;
			}
			escape = false;
			break;

		case '\\':
			escape = true;
			buffer += input.get();
			positionCounter++;
			break;

		case '\n': 
			lineCounter++;
			positionCounter = 0;

		default: // end of whitespace, stop please
			escape = false;
			buffer += input.get();
			positionCounter++;
			break;

			// all chars should be handled
		case  std::char_traits<char>::eof():
			token = Token::FileEnd;
			parsing = false;
			throw std::invalid_argument("File ended before string finished.");
			break;

		}
	}
	// type detection
	token = Token::String;
	if (buffer.size() > 2) {
		if (buffer[1] == '"') {
			switch (buffer[0]) {
			case 'b':
			case 'B':
				token = Token::StringBinary;
				break;
			case 'l':
			case 'L':
				token = Token::StringUnicode;
				break;
			}
		}
	}
}


// parse in a whitespace, consuming character from input 
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

void Carbon::Compiler::Lexer::ParseOperator()
{
	// first char peek
	auto continueParsing = false;
	auto acceptFirst = true;
	switch (input.peek()) {
	case '+':
		token = Token::Plus;
		break;
	case '-':
		token = Token::Minus;
		break;
	case '<':
		token = Token::Less;
		continueParsing = true; // need to check next char, might be <=
		break;
	case '>':
		token = Token::Greater;
		continueParsing = true; // might be >=
		break;
		break;
	case '!':
		token = Token::Not;
		continueParsing = true; // might be !=
		break;
	case '*':
		token = Token::Multiply;
		break;
	case '/':
		token = Token::Divide;
		break;
	case '.':
		token = Token::Member;
		break;
	case ';': 
		token = Token::EndStatement;
		break;
	case '=':
		token = Token::Assign;
		continueParsing = true; // might be ==
		break;
	default:
		acceptFirst = false;
		break;
	}
	// first char has been accepted
	if (acceptFirst) {
		positionCounter += 1;
		input.get();
	}
	if (!continueParsing)
	{
		// single char operators exit here
		return;
	}
	// second char for two char operators
	continueParsing = false;
	auto acceptSecond = true;
	switch (input.peek())
	{
	case '=':
		if (token == Token::Assign) { // equals operator == 
			token = Token::Equals;
		}
		else if (token == Token::Not) {
			token = Token::NotEquals;
		}
		else if (token == Token::Less) {
			token = Token::LessOrEqual;
		}
		else if (token == Token::Greater) {
			token = Token::GreaterOrEqual;
		}
		else {
			acceptSecond = false;
		}
		break;
	default:
		acceptSecond = false;
		break;
	}
	if (acceptSecond) 
	{
		positionCounter += 1;
		input.get();
	}
	if (!continueParsing) 
	{
		// operators with 1 or 2 chars exit here
		return;
	}
	// if required continue here for 3 char operators
	throw std::logic_error("This line should never execute.");
}

Carbon::Compiler::Token Carbon::Compiler::Lexer::DetectKeyword(const std::string& token)
{
	// needs to be verified of keywords are added or changed
	const int minTokenLength = 2; // min size of token, used to filter out bad cases
	const int maxTokenLength = 8; // max size of token, used to filter out definitely not keywords
	auto length = token.length();
	if (length<minTokenLength || length>maxTokenLength) {
		return Token::Id; //definitely not a keyword
	}
	// keywords are indexed by first char
	switch (token[0]) {
	case 'f':
		if (token == "function") {
			return Token::Function;
		} 
		break;
	case 'l':
		if (token == "local") {
			return Token::Local;
		} else if (token == "loop") {
			return Token::Loop;
		}
		break;
	case 'e':
		if (token == "else") {
			return Token::Else;
		}
		break;
	case 'b':
		if (token == "break") {
			return Token::Break;
		}
		break;
	case 'c':
		if (token == "continue") {
			return Token::Continue;
		}
		break;
	case 'r':
		if (token == "return") {
			return Token::Return;
		}
		break;
	case 'i':
		if (token == "if") {
			return Token::If;
		}
		break;
	}
	return Token::Id; //definitely not a keyword
}

// start of a token, reset buffer and store position
void Carbon::Compiler::Lexer::StartToken()
{
	tokenStartLine = lineCounter;
	tokenStartPosition = positionCounter;
	buffer = "";
}

// read next token
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
			StartToken();
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
			StartToken();
			ParseIdOrString();
			parsing = false;
			break;

		case '"':
			StartToken();
			ParseString();
			parsing = false;
			break;

		case '+': case '-': case '*': case '/': 
		case '=': case '!': case '<': case '>':
		case '&': case '|': case '^': case '~':
		case '(': case ')': case '{': case '}':
		case '[': case ']': case '.': case ',':
		case ';':
			StartToken();
			ParseOperator();
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


