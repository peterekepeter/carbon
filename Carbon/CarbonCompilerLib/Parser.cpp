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
	bool parsing = true;
	while (parsing)
	switch (state.top()) {
	case State::Program:
		parsing = ParseProgram();
		break;
	case State::Statement:
		parsing = ParseStatement();
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
	return lexer.GetData();
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
		return false;	
	default:
		throw ParseError();
		break;
	}
}

std::invalid_argument Carbon::Compiler::Parser::ParseError()
{
	std::stringstream ss;
	ss << "Unexpected" << lexer.GetData() << " line " << lexer.GetLine() << " position " << lexer.GetPosition() << "\n";
	return std::invalid_argument(ss.str());
}
