#include "Parser.h"

Carbon::Compiler::Parser::Parser(Lexer & lexer) : lexer(lexer)
{
	state.push(State::Program);
}

bool Carbon::Compiler::Parser::MoveNext()
{
	if (state.size() == 0) {
		// invalid parser state, probably finished file already
		return false;
	}
	instructionData.clear();
	bool parsing = true;
	while (parsing)
	switch (state.top()) {
	case State::Program:
		parsing = ParseProgram();
		break;
	case State::Statement:
		parsing = ParseStatement();
		break;
	case State::BlockOrObject:
		parsing = ParseBlockOrObject();
		break;
	case State::BlockTemp:
		parsing = ParseBlock();
		break;
	case State::BlockOrObject2ndToken:
		parsing = ParseBlockOrObject2ndToken();
		break;
	case State::Expression:
		parsing = ParseExpression();
		break;
	default:
		throw std::runtime_error("Unexpected compiler state.");
		break;
	}
	// return true while parser has valid state
	return state.size() > 0;
}

InstructionType Carbon::Compiler::Parser::ReadInstructionType()
{
	return this->instruction;
}

const char * Carbon::Compiler::Parser::ReadStringData()
{
	return this->instructionData.c_str();
}

bool Carbon::Compiler::Parser::ParseProgram()
{
	switch (lexer.GetToken()) {
	case Token::FileBegin:
		// file just begin, totally accetable, move to next token
		lexer.MoveNext();
		return true; // continue parsing
	case Token::FileEnd:
		// file just ended, we're in program state, so return true, no error
		// this is the expected normal termination of a program
		state.pop();
		return false; //yield
	case Token::EndStatement:
		// found an extra end statement, just filter it out, no need to complicate
		lexer.MoveNext();
		return true; // continue parsing
	default: // any other token would indicate the beginning of a statement
		state.push(State::Statement);
		return true; // continue parsing
	}
}

bool Carbon::Compiler::Parser::ParseStatement()
{
	switch (lexer.GetToken()) {
	case Token::EndStatement:
		// end of the statement
		lexer.MoveNext();
		state.pop();
		// found instruction
		instruction = InstructionType::END_STATEMENT;
		return false; // yield end statement
	case Token::BracesOpen:
		lexer.MoveNext();
		state.pop();
		state.push(State::BlockOrObject);
		return true; // continue parsing
	case Token::Function: // function keyword
	case Token::Local: // variable declaration keyword
	// also for all atoms
	case Token::Id:
	case Token::String:
	case Token::StringBinary:
	case Token::StringUnicode:
	case Token::Float:
	case Token::Number:
	case Token::NumberBinary:
	case Token::NumberHexadecimal:
	case Token::NumberOctal:
		state.push(State::Expression); // start parsing expression
		return true; //continue parsing
	case Token::FileEnd:
		state.pop();
		instruction = InstructionType::END_STATEMENT;
		return false; // yield end statement
	default:
		throw ParseError();
		break;
	}
}

bool Carbon::Compiler::Parser::ParseBlockOrObject()
{
	switch (lexer.GetToken())
	{
	case Token::Id: // can be object like { x: 3, ...
		tempBuffer = lexer.GetData();
		tempToken = Token::Id;
		lexer.MoveNext();
		state.pop();
		state.push(State::BlockOrObject2ndToken);
		return true; // continue parsing
	case Token::String: // can be object like { "x": 3, ...
		tempBuffer = lexer.GetData();
		tempToken = Token::String;
		lexer.MoveNext();
		state.pop();
		state.push(State::BlockOrObject2ndToken);
		return true; // continue parsing
	default:
		// ok definitely not an object
		state.pop();
		state.push(State::Block);
		instruction = InstructionType::BLOCKBEGIN;
		return false; // yield
	}
}

bool Carbon::Compiler::Parser::ParseBlockOrObject2ndToken()
{
	switch (lexer.GetToken())
	{
	case Token::Colon:
		// deinitely object
		state.pop();
		state.push(State::ObjectTemp); // temp token we need to treat before resuming state object
		instruction = InstructionType::OBJECTBEGIN;
		return false; // yield OBJECTBEGIN
	default:
		// ok definitely not an object
		state.pop();
		state.push(State::BlockTemp); // temp token we need to treat before resuming state block
		instruction = InstructionType::BLOCKBEGIN;
		return false; // yield OBJECTBEGIN
	}
}

bool Carbon::Compiler::Parser::ParseBlockTemp()
{
	state.pop(); // clear blocktemp
	state.push(State::Block); // push block
	state.push(State::Expression); // and since we have an id or string, we're in expression
	if (tempToken == Token::Id) {
		instruction = InstructionType::ID;
	}
	else if (tempToken == Token::String) {
		instruction = InstructionType::STR;
	} else {
		throw std::runtime_error("unexpected parser state");
	}
	this->instructionData = tempBuffer.c_str();
	return false; //yield ATOM
}

bool Carbon::Compiler::Parser::ParseBlock()
{
	switch (lexer.GetToken()) {
	case Token::FileBegin:
		// file just begin, totally accetable, move to next token
		lexer.MoveNext();
		return true; // continue parsing
	case Token::BracesClose:
		state.pop();
		instruction = InstructionType::BLOCKEND;
		return false; // yield
	case Token::FileEnd:
		// expecting block to finish
		throw ParseError("Code block is not closed, unexpected end of file");
		return false; //yield
	case Token::EndStatement:
		// found an extra end statement, just filter it out, no need to complicate
		lexer.MoveNext();
		return true; // continue parsing
	default: // any other token would indicate the beginning of a statement
		state.push(State::Statement);
		return true; // continue parsing
	}
}

bool Carbon::Compiler::Parser::ParseObject()
{
	return false;
}

bool Carbon::Compiler::Parser::ParseObjectTemp()
{
	return false;
}

InstructionType Carbon::Compiler::Parser::TokenToInfixOperator(Token top) const
{
	switch (top)
	{
	case Token::Assign: return InstructionType::ASSIGN;
	case Token::Plus: return InstructionType::ADD;
	case Token::Minus: return InstructionType::SUBTRACT;
	case Token::Multiply: return InstructionType::MULTIPLY;
	case Token::Divide: return InstructionType::DIVIDE;
	case Token::Equals: return InstructionType::COMP_EQ;
	case Token::NotEquals: return InstructionType::COMP_NE;
	case Token::Greater: return InstructionType::COMP_GT;
	case Token::GreaterOrEqual: return InstructionType::COMP_GE;
	case Token::Less: return InstructionType::COMP_LT;
	case Token::LessOrEqual: return InstructionType::COMP_LE;
	default:
		throw std::runtime_error("unexpected parser state");
	}
}

bool Carbon::Compiler::Parser::ParseExpression()
{
	switch (auto token = lexer.GetToken())
	{
		// atom in expression
	case Token::Id:
	case Token::String:
	case Token::StringBinary:
	case Token::StringUnicode:
	case Token::Float:
	case Token::Number:
	case Token::NumberBinary:
	case Token::NumberHexadecimal:
	case Token::NumberOctal:
		instruction = TokenToAtom(token);
		instructionData = lexer.GetData();
		lexer.MoveNext(); // consume token
		return false; // yield
	// operators
	case Token::Assign: {
		bool push = true;
		if (opStack.size() > 0)
		{
			auto precedenceToken = TokenPrecedence(token);
			auto precedenceOpstackTop = TokenPrecedence(opStack.top());
			if (precedenceToken < precedenceOpstackTop)
			{
				push = false;
			}
			else if (precedenceToken == precedenceOpstackTop && !TokenIsRightAssociative(token))
			{
				push = false;
			}
		}
		if (push)
		{
			opStack.push(token);
			lexer.MoveNext(); // consume token
			return true; //continue parsing
		}
		else
		{
			instruction = TokenToInfixOperator(opStack.top());
			opStack.pop();
			return false;
		}
	}
		break;
	case Token::FileEnd:
	case Token::EndStatement:
		if (opStack.size()>0)
		{
			instruction = TokenToInfixOperator(opStack.top());
			opStack.pop();
			return false; // yield
		}
		else
		{
			state.pop();
			return true; // yield
		}
		break;
	default:
		throw ParseError();
		break;
	}
}

InstructionType Carbon::Compiler::Parser::TokenToAtom(Token token)
{
	switch (token)
	{
	case Token::Id:	
		return InstructionType::ID; 
	case Token::String:				
		return InstructionType::STR;
	case Token::StringBinary:		
		return InstructionType::BSTR;
	case Token::StringUnicode:		
		return InstructionType::USTR; 
	case Token::Float:				
		return InstructionType::FLOAT; 
	case Token::Number:				
		return InstructionType::NUM;
	case Token::NumberBinary:		
		return InstructionType::BNUM; 
	case Token::NumberHexadecimal:	
		return InstructionType::XNUM; 
	case Token::NumberOctal:		
		return InstructionType::ONUM;
	default: 
		throw ParseError(); break;
	}
}

std::invalid_argument Carbon::Compiler::Parser::ParseError()
{
	std::stringstream ss;
	ss << "Unexpected" << lexer.GetData() << " at line " << lexer.GetLine() << " position " << lexer.GetPosition() << "\n";
	return std::invalid_argument(ss.str());
}

std::invalid_argument Carbon::Compiler::Parser::ParseError(const char* message)
{
	std::stringstream ss;
	ss << message << " at line " << lexer.GetLine() << " position " << lexer.GetPosition() << "\n";
	return std::invalid_argument(ss.str());
}

