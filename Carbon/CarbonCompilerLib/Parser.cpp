#include "Parser.h"

// more or less the following is implemented here
// 
// %right '='
// %left TOK_OP_EQ TOK_OP_NE
// %left '<' '>' TOK_OP_LE TOK_OP_GE
// %left '+' '-'
// %left '*' '/'
// %left '.'
// 
// %%
// 
// program : //nothing    
//         | program statement 
//         ;
// 
// statement : '{' {INS_CMD(BLOCKBEGIN);} program '}' {INS_CMD(BLOCKEND);}
//           | TOK_KEY_LOOP {INS_CMD(CONTROL);} loop 
//           | TOK_KEY_IF {INS_CMD(CONTROL);} '(' expression ')' statement else
//           | expression TOK_ENDST  { INS_CMD(END_STATEMENT); }
//           | TOK_KEY_RETURN TOK_ENDST { INS_CMD(RETURN0); }
//           | TOK_KEY_RETURN expression TOK_ENDST { INS_CMD(RETURN1); }
//           | TOK_KEY_BREAK TOK_ENDST { INS_CMD(BREAK); }
//           | TOK_KEY_CONTINUE TOK_ENDST { INS_CMD(CONTINUE); }
//           | TOK_ENDST
//           ;
// 
// else: TOK_KEY_ELSE statement {INS_CMD(IFELSE);} 
//     | {INS_CMD(IF);}
//     ;
// 
// loop: statement {INS_CMD(LOOP0);}
//     | '(' expression ')' statement {INS_CMD(LOOP1);}
//     | '(' expression ',' expression ')' statement {INS_CMD(LOOP2);} 
//     | '(' expression ',' expression ',' expression ')' statement {INS_CMD(LOOP3);} 
//     ;
// 
// 
// expression: TOK_KEY_FUNCTION '(' { INS_CMD(FUNCTIONBEGIN); } idlist ')' statement { INS_CMD(FUNCTIONEND); }
//           | TOK_KEY_LOCAL expression { INS_CMD(LOCAL); }
//           | atom '(' { INS_CMD(CALLBEGIN); } calllist ')' { INS_CMD(CALLEND); }
// 		  | '{' { INS_CMD(OBJECTBEGIN); } keyvaluelist '}' { INS_CMD(OBJECTEND); }
// 		  | '[' { INS_CMD(ARRAYBEGIN); } calllist ']' { INS_CMD(ARRAYEND); }
//           | expression '=' expression       { INS_CMD(ASSIGN); }
//           | expression TOK_OP_EQ expression { INS_CMD(COMP_EQ);} 
//           | expression TOK_OP_NE expression { INS_CMD(COMP_NE);} 
//           | expression TOK_OP_LE expression { INS_CMD(COMP_LE);} 
//           | expression TOK_OP_GE expression { INS_CMD(COMP_GE);} 
//           | expression '<' expression { INS_CMD(COMP_LT);} 
//           | expression '>' expression { INS_CMD(COMP_GT);} 
//           | expression '+' expression { INS_CMD(ADD);} 
//           | expression '-' expression { INS_CMD(SUBTRACT); }
//           | expression '*' expression { INS_CMD(MULTIPLY); }
//           | expression '/' expression { INS_CMD(DIVIDE); }
// 		  | expression '.' expression { INS_CMD(MEMBER); }
//           | '+' expression            { INS_CMD(POSITIVE); }
//           | '-' expression            { INS_CMD(NEGATIVE); }
//           | '(' expression ')'
//           | atom 
//           ;
// 
// keyvaluelist: //nothing
// 			| keyvaluelist ',' objectkey ':' expression  
// 			| objectkey ':' expression
// 			;
// 
// objectkey: TOK_ID { INS_ATOM(ID); }
// 		 | TOK_STR { INS_ATOM(STR); }
// 		 ;
// 
// idlist: //nothing
//       | idlist ',' identifier
//       | identifier
//       ;
// 
// identifier: TOK_ID { INS_ATOM(ID); }
//           ;
// 
// calllist: //nothing
//         | calllist ',' expression
//         | expression
//         ;
// 
// atom: TOK_USTR          { INS_ATOM(USTR); }
//     | TOK_XSTR          { INS_ATOM(XSTR); }
//     | TOK_BSTR          { INS_ATOM(BSTR); }
//     | TOK_STR           { INS_ATOM(STR); }
//     | TOK_ID            { INS_ATOM(ID); }
//     | TOK_FLOAT         { INS_ATOM(FLOAT); }
//     | TOK_XNUM          { INS_ATOM(XNUM); }
//     | TOK_BNUM          { INS_ATOM(BNUM); }
//     | TOK_ONUM          { INS_ATOM(ONUM); }
//     | TOK_NUM           { INS_ATOM(NUM); }
//     ;
// 
// %%

std::string Carbon::ParserException::GetMessage() const
{
	return this->what();
}

std::string Carbon::ParserException::GetMessageWithLineAndPosition() const
{
	std::stringstream ss;
	ss << this->what() << " at line " << this->Line << " position " << this->LinePosition << ".";
	return ss.str();
}

Carbon::ParserException::ParserException(const std::string& _Message): logic_error(_Message), Line(-1), LinePosition(-1)
{
}

Carbon::ParserException::ParserException(const char* _Message): logic_error(_Message), Line(-1), LinePosition(-1)
{
}

Carbon::Parser::Parser(Lexer & lexer) : lexer(lexer)
{
	state.push(State::Program);
}

bool Carbon::Parser::MoveNext()
{
	if (state.size() == 0) {
		// invalid parser state, probably finished file already
		return false;
	}
	instructionData.clear();
	bool parsing = true;
	while (parsing) {
		// advance lexer if needed, this is done here so that REPL can return at end of expression
		if (consumeToken == true) {
			// the parser has previouselt signaled to advance lexer
			lexer.MoveNext();
			consumeToken = false; // the flag  needs to be set again
		}
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
			parsing = ParseBlockTemp();
			break;
		case State::Block:
			parsing = ParseBlock();
			break;
		case State::BlockOrObject2ndToken:
			parsing = ParseBlockOrObject2ndToken();
			break;
		case State::Expression:
			parsing = ParseExpression();
			break;
		case State::ExpressionPopUnary:
			parsing = ParseExpressionPopUnary();
			break;
		case State::KeyValue:
			parsing = ParseKeyValue();
			break;
		case State::KeyValueColon:
			parsing = ParseKeyValueColon();
			break;
		case State::KeyValueComma:
			parsing = ParseKeyValueComma();
			break;
		case State::CallList:
			parsing = ParseCallList();
			break;
		case State::CallListComma:
			parsing = ParseCallListComma();
			break;
		case State::IdList:
			parsing = ParseIdList();
			break;
		case State::IdListComma:
			parsing = ParseIdListComma();
			break;
		case State::FunctionBegin:
			parsing = ParseFunctionBegin();
			break;
		case State::FunctionEndParameters:
			parsing = ParseFunctionEndParameters();
			break;
		case State::FunctionEnd:
			parsing = ParseFunctionEnd();
			break;
		case State::LocalEnd:
			parsing = ParseLocalEnd();
			break;
		case State::BreakStatement:
			parsing = ParseBreakStatement();
			break;
		case State::ContinueStatement:
			parsing = ParseContinueStatement();
			break;
		case State::ReturnStatement:
			parsing = ParseReturnStatement();
			break;
		case State::ReturnStatementWithExpression:
			parsing = ParseReturnStatementWithExpression();
			break;
		case State::IfBeginParam:
			parsing = ParseIfBeginParam();
			break;
		case State::IfEndParam:
			parsing = ParseIfEndParam();
			break;
		case State::IfElse:
			parsing = ParseIfElse();
			break;
		case State::IfElseEnd:
			parsing = ParseIfElseEnd();
			break;
		case State::LoopBegin:
			parsing = ParseLoopBegin();
			break;
		case State::LoopEnd0:
			parsing = ParseLoopEnd0();
			break;
		case State::LoopEnd1:
			parsing = ParseLoopEnd1();
			break;
		case State::LoopEnd2:
			parsing = ParseLoopEnd2();
			break;
		case State::LoopEnd3:
			parsing = ParseLoopEnd3();
			break;
		case State::LoopParamBegin:
			parsing = ParseLoopParamBegin();
			break;
		case State::LoopParamComma:
			parsing = ParseLoopParamComma();
			break;
		case State::LoopParamSecond:
			parsing = ParseLoopParamSecond();
			break;
		case State::LoopParamSecondComma:
			parsing = ParseLoopParamSecondComma();
			break;
		case State::LoopParamThird:
			parsing = ParseLoopParamThird();
			break;
		case State::LoopParamThirdEnd:
			parsing = ParseLoopParamThirdEnd();
			break;
		default:
			throw std::runtime_error("Unexpected compiler state.");
			break;
		}
	}
	// return true while parser has valid state
	return state.size() > 0;
}

Carbon::InstructionType Carbon::Parser::ReadInstructionType()
{
	return this->instruction;
}

const char * Carbon::Parser::ReadStringData()
{
	return this->instructionData.c_str();
}

bool Carbon::Parser::ParseProgram()
{
	switch (lexer.GetToken()) {
	case Token::FileBegin:
		// file just begin, totally accetable, move to next token
		consumeToken = true;
		return true; // continue parsing
	case Token::BracesClose:
		// we're in a code block, upper level should handle braces token
		state.pop();
		return true; //continue parsing
	case Token::FileEnd:
		// file just ended, we're in program state, so return true, no error
		// this is the expected normal termination of a program
		state.pop();
		return false; //yield
	case Token::EndStatement:
		// found an extra end statement, just filter it out, no need to complicate
		consumeToken = true;
		return true; // continue parsing
	default: // any other token would indicate the beginning of a statement
		state.push(State::Statement);
		return true; // continue parsing
	}
}

bool Carbon::Parser::ParseStatement()
{
	switch (lexer.GetToken()) {
	case Token::EndStatement:
		// end of the statement
		consumeToken = true;
		state.pop();
		// found instruction
		instruction = InstructionType::END_STATEMENT;
		return false; // yield end statement

	case Token::BracesOpen:
		consumeToken = true;
		state.pop();
		state.push(State::BlockOrObject);
		return true; // continue parsing

	case Token::Loop:
		consumeToken = true; // consume loop keyword
		state.push(State::LoopBegin);
		instruction = InstructionType::CONTROL;
		return false; // yield control instruction

	case Token::If:
		consumeToken = true; // consume if token
		instruction = InstructionType::CONTROL;
		state.push(State::IfElse);
		state.push(State::Statement);
		state.push(State::IfEndParam);
		state.push(State::Expression);
		opStack.push(Op::Expression);
		state.push(State::IfBeginParam);
		return false; // yield control instruction


	case Token::Break:
		state.push(State::BreakStatement);
		consumeToken = true; // consume break token
		return true; // continue parsing

	case Token::Continue:
		state.push(State::ContinueStatement);
		consumeToken = true; // consume break token
		return true; // continue parsing

	case Token::Return:
		state.push(State::ReturnStatement);
		consumeToken = true; // consume break token
		return true;

	// expression follows
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
	case Token::Minus:
	case Token::Plus:
		state.push(State::Expression); // start parsing expression
		opStack.push(Op::Expression);
		return true; //continue parsing

	// file end
	case Token::FileEnd:
	case Token::BracesClose:
		state.pop();
		instruction = InstructionType::END_STATEMENT;
		return false; // yield end statement
	default:
		throw ParseError();
		break;
	}
}

bool Carbon::Parser::ParseBlockOrObject()
{
	switch (lexer.GetToken())
	{
	case Token::Id: // can be object like { x: 3, ...
		tempBuffer = lexer.GetData();
		tempToken = Token::Id;
		consumeToken = true;
		state.pop();
		state.push(State::BlockOrObject2ndToken);
		return true; // continue parsing
	case Token::String: // can be object like { "x": 3, ...
		tempBuffer = lexer.GetData();
		tempToken = Token::String;
		consumeToken = true;
		state.pop();
		state.push(State::BlockOrObject2ndToken);
		return true; // continue parsing
	default:
		// ok definitely not an object
		state.pop();
		state.push(State::Block);
		state.push(State::Program);
		instruction = InstructionType::BLOCKBEGIN;
		return false; // yield
	}
}

bool Carbon::Parser::ParseBlockOrObject2ndToken()
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

bool Carbon::Parser::ParseBlockTemp()
{
	state.pop(); // clear blocktemp
	state.push(State::Block); // push block
	state.push(State::Program);
	state.push(State::Statement);
	state.push(State::Expression); // and since we have an id or string, we're in expression
	opStack.push(Op::Expression);
	if (tempToken == Token::Id) {
		instruction = InstructionType::ID;
		opStack.push(Op::Term);
	}
	else if (tempToken == Token::String) {
		instruction = InstructionType::STR;
		opStack.push(Op::Term);
	} else {
		throw std::runtime_error("unexpected parser state");
	}
	this->instructionData = tempBuffer;
	return false; //yield ATOM
}

bool Carbon::Parser::ParseBlock()
{
	switch (lexer.GetToken()) {
	case Token::FileBegin:
		// file just begin, totally accetable, move to next token
		consumeToken = true;
		return true; // continue parsing
	case Token::BracesClose:
		state.pop();
		instruction = InstructionType::BLOCKEND;
		consumeToken = true; // consume braces close token
		return false; // yield
	case Token::FileEnd:
		// expecting block to finish
		throw ParseError("Code block is not closed, unexpected end of file");
		return false; //yield
	case Token::EndStatement:
		// found an extra end statement, just filter it out, no need to complicate
		consumeToken = true;
		return true; // continue parsing
	default: // any other token would indicate the beginning of a statement
		state.push(State::Statement);
		return true; // continue parsing
	}
}

bool Carbon::Parser::ParseKeyValue()
{
	switch (auto token = lexer.GetToken())
	{
	case Token::Id:
	case Token::String:
		state.pop();
		state.push(State::KeyValueColon);
		instruction = TokenToAtom(token);
		instructionData = lexer.GetData();
		consumeToken = true; // consume token
		return false; // yield
	case Token::BracesClose:
		state.pop();
		return true; // continue parsing
	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseKeyValueColon()
{
	// expecting ':'
	switch (lexer.GetToken())
	{
	case Token::Colon:
		// OK
		consumeToken = true; // consume colon
		state.pop();
		state.push(State::KeyValueComma);
		state.push(State::Expression);
		opStack.push(Op::Expression);
		return true; // continue parsing expression
	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseKeyValueComma()
{
	// expecting ',' or '}'
	switch (lexer.GetToken())
	{
	case Token::Comma:
		// comma found, keyvalue continues
		consumeToken = true; // consume comma
		state.pop();
		state.push(State::KeyValue);
		return true; //continue parsing
	case Token::BracesClose: // end
		state.pop();
		return true; //continue parsing
	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseCallList()
{
	switch (lexer.GetToken())
	{
	case Token::ParanthesisClose:
		// paranthesis should be on top of stack
		if (opStack.top() == Op::FunctionCall)
		{
			// ok, closing call list 
			state.pop();
			return true; // continue parsing
		}
		throw ParseError("Unexpected parser state.");

	case Token::BracketClose:
		// bracket should be on top of stack
		if (opStack.top() == Op::Bracket)
		{
			// ok, closing array call list
			state.pop();
			return true; // continue parsing
		}
		throw ParseError("Unexpected parser state.");

	case Token::BracesOpen:
	case Token::BracketOpen:
	case Token::ParanthesisOpen:
	case Token::Id:
	case Token::String:
	case Token::StringBinary:
	case Token::StringUnicode:
	case Token::Float:
	case Token::Number:
	case Token::NumberBinary:
	case Token::NumberHexadecimal:
	case Token::NumberOctal:
	case Token::Minus:
	case Token::Plus:
		// it's an expression
		state.pop();
		state.push(State::CallListComma);
		state.push(State::Expression);
		opStack.push(Op::Expression);
		return true;

	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseCallListComma()
{
	// expecting a comma, then continue with call list
	switch (lexer.GetToken())
	{
	case Token::ParanthesisClose:
		// paranthesis should be on top of stack
		if (opStack.top() == Op::FunctionCall)
		{
			// ok, closing call list
			state.pop();
			return true; // continue parsing
		}
		throw ParseError("Unexpected parser state.");

	case Token::BracketClose:
		// bracket should be on top of stack
		if (opStack.top() == Op::Bracket)
		{
			// ok, closing array call list
			state.pop();
			return true; // continue parsing
		}
		throw ParseError("Unexpected parser state.");

	case Token::Comma:
		state.pop();
		state.push(State::CallList);
		consumeToken = true; // consume token
		return true; // continue parsing

	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseIdList()
{
	switch (lexer.GetToken())
	{
	case Token::ParanthesisClose:
		// paranthesis should be on top of stack
		if (opStack.top() == Op::Paranthesis)
		{
			// ok, closing id list 
			state.pop();
			return true; // continue parsing
		}
		throw ParseError("Unexpected parser state.");

	case Token::Id:
		instruction = InstructionType::ID;
		instructionData = lexer.GetData();
		consumeToken = true; // consume id
		state.pop();
		state.push(State::IdListComma);
		return false; // yield id

	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseIdListComma()
{
	// expecting a comma, then continue with id list
	switch (lexer.GetToken())
	{
	case Token::ParanthesisClose:
		// paranthesis should be on top of stack
		if (opStack.top() == Op::Paranthesis)
		{
			// ok, closing id list
			state.pop();
			return true; // continue parsing
		}
		throw ParseError("Unexpected parser state.");

	case Token::Comma:
		state.pop();
		state.push(State::IdList);
		consumeToken = true; // consume comma
		return true; // continue parsing

	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseFunctionEnd()
{
	state.pop();
	instruction = InstructionType::FUNCTIONEND;
	return false;
}

bool Carbon::Parser::ParseFunctionBegin()
{
	switch (lexer.GetToken())
	{
	case Token::ParanthesisOpen:
		consumeToken = true; // consume parathesis
		state.pop();
		state.push(State::FunctionEndParameters);
		opStack.push(Op::Paranthesis); // for idlist
		state.push(State::IdList);
		return true; // continue with id list
	default:
		throw ParseError("Expecting function parameter list opening paranthesis");
	}
}

bool Carbon::Parser::ParseFunctionEndParameters()
{
	switch (lexer.GetToken())
	{
	case Token::ParanthesisClose:
		// check
		if (opStack.top() != Op::Paranthesis)
		{
			throw ParseError();
		}
		consumeToken = true; // consume parathesis
		state.pop();
		opStack.pop();
		state.push(State::FunctionEnd);
		state.push(State::Statement);
		return true; // continue with id list
	default:
		throw ParseError("Expecting function parameter list closing paranthesis");
	}
}

bool Carbon::Parser::ParseLocalEnd()
{
	state.pop();
	instruction = InstructionType::LOCAL;
	return false;
}

bool Carbon::Parser::ParseBreakStatement()
{
	switch (lexer.GetToken())
	{
	case Token::EndStatement:
		instruction = InstructionType::BREAK;
		state.pop();
		return false;
	default:
		throw ParseError("Break keyword should be followed by semicolon");
	}
}

bool Carbon::Parser::ParseContinueStatement()
{
	switch (lexer.GetToken())
	{
	case Token::EndStatement:
		instruction = InstructionType::CONTINUE;
		state.pop();
		return false;
	default:
		throw ParseError("Continue keyword should be followed by semicolon");
	}
}

bool Carbon::Parser::ParseReturnStatement()
{
	switch (lexer.GetToken())
	{
	case Token::BracesClose:
	case Token::FileEnd:
	case Token::EndStatement:
		instruction = InstructionType::RETURN0;
		state.pop(); // go back to statement which handles the end statement token
		return false; // yield return instruction

	default: // must be a return with an expression
		state.pop();
		state.push(State::ReturnStatementWithExpression);
		state.push(State::Expression);
		opStack.push(Op::Expression);
		return true; // continue with parsing the expression
	}
}

bool Carbon::Parser::ParseReturnStatementWithExpression()
{
	switch (lexer.GetToken())
	{
	case Token::BracesClose:
	case Token::FileEnd:
	case Token::EndStatement: // there should be an end statement
		instruction = InstructionType::RETURN1;
		state.pop(); // go back to statement which handles the end statement token
		return false;
	default:
		throw ParseError("Expecting semicolon to terminate return statement");
	}
}

bool Carbon::Parser::ParseIfBeginParam()
{
	switch(lexer.GetToken())
	{
	case Token::ParanthesisOpen:
		// if parameter begins when paranthesis opens
		consumeToken = true; // consume token
		state.pop(); // goto Expression
		return true; // continue parsing
	default:
		// no other tokens are accepted in this state
		throw ParseError("Expecting opening paranthesis for condition");
	}
}

bool Carbon::Parser::ParseIfEndParam()
{
	switch (lexer.GetToken())
	{
	case Token::ParanthesisClose:
		// if parameter ends when paranthesis closes
		consumeToken = true; // consume token
		state.pop(); // goto statement
		return true; // continue parsing
	default:
		// no other tokens are accepted in this state
		throw ParseError("Expecting opening paranthesis for condition");
	}
}

bool Carbon::Parser::ParseIfElse()
{
	// case: we're in an if statement, after the implementation of then branch
	switch (lexer.GetToken())
	{
	case Token::Else:
		// if we find an else token here then we continue
		// with the else branch of this if else statement
		consumeToken = true; // consume else token 
		state.pop();
		state.push(State::IfElseEnd);
		state.push(State::Statement); //else branch
		return true; // continue parsing
	default:
		instruction = InstructionType::IF; // we only have if then, no else
		state.pop(); // on any other character should be handled by upper level
		return false; // yield if
	}
}

bool Carbon::Parser::ParseIfElseEnd()
{
	// we're at the end of an ifelse instruction, 
	// then and else branches have been implemented
	instruction = InstructionType::IFELSE;
	state.pop();
	return false; // yield ifelse instruction
}

bool Carbon::Parser::ParseLoopBegin()
{
	switch (lexer.GetToken())
	{
	case Token::BracesOpen:
		state.pop();
		state.push(State::LoopEnd0);
		state.push(State::Statement); // it's a statement
		return true; // continue parsing
	case Token::ParanthesisOpen:
		state.pop();
		state.push(State::LoopParamBegin);
		consumeToken = true; // consume parantehsis open token
		return true; // continue parsing
	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseLoopEnd0()
{
	instruction = InstructionType::LOOP0;
	state.pop();
	return false; // yield loop instruction
}

bool Carbon::Parser::ParseLoopParamBegin()
{
	switch(lexer.GetToken())
	{
	case Token::Comma:
		throw ParseError();
	default:
		state.pop();
		state.push(State::LoopParamComma);
		state.push(State::Expression);
		opStack.push(Op::Expression);
		return true; // continue parsing
	}
}

bool Carbon::Parser::ParseLoopParamComma()
{
	switch (lexer.GetToken())
	{
	case Token::Comma:
		state.pop();
		consumeToken = true; // consume token
		state.push(State::LoopParamSecond);
		return true; // continue parsing
	case Token::ParanthesisClose:
		state.pop();
		state.push(State::LoopEnd1);
		state.push(State::Statement); // it's a statement
		consumeToken = true; // consume token
		return true; // continue parsing
	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseLoopParamSecond()
{
	switch (lexer.GetToken())
	{
	case Token::Comma:
		throw ParseError();
	default:
		state.pop();
		state.push(State::LoopParamSecondComma);
		state.push(State::Expression);
		opStack.push(Op::Expression);
		return true; // continue parsing
	}
}

bool Carbon::Parser::ParseLoopParamSecondComma()
{
	switch (lexer.GetToken())
	{
	case Token::Comma:
		state.pop();
		consumeToken = true; // consume token
		state.push(State::LoopParamThird);
		return true; // continue parsing
	case Token::ParanthesisClose:
		state.pop();
		state.push(State::LoopEnd2);
		state.push(State::Statement); // it's a statement
		consumeToken = true; // consume token
		return true; // continue parsing
	default:
		throw ParseError();
	}
}

bool Carbon::Parser::ParseLoopEnd1()
{
	instruction = InstructionType::LOOP1;
	state.pop();
	return false; // yield loop instruction
}

bool Carbon::Parser::ParseLoopParamThird()
{
	switch (lexer.GetToken())
	{
	case Token::Comma:
		throw ParseError();
	default:
		state.pop();
		state.push(State::LoopParamThirdEnd);
		state.push(State::Expression);
		opStack.push(Op::Expression);
		return true; // continue parsing
	}
}

bool Carbon::Parser::ParseLoopEnd2()
{
	instruction = InstructionType::LOOP2;
	state.pop();
	return false; // yield loop instruction
}

bool Carbon::Parser::ParseLoopParamThirdEnd()
{
	switch (lexer.GetToken())
	{
	case Token::ParanthesisClose:
		state.pop();
		state.push(State::LoopEnd3);
		state.push(State::Statement); // it's a statement
		consumeToken = true; // consume token
		return true; // continue parsing
	default:
		throw ParseError();
	}

}

bool Carbon::Parser::ParseLoopEnd3()
{
	instruction = InstructionType::LOOP3;
	state.pop();
	return false; // yield loop instruction
}

Carbon::InstructionType Carbon::Parser::OpToInstructionType(Op top) const
{
	switch (top)
	{
	case Op::Assign: return InstructionType::ASSIGN;
	case Op::Add: return InstructionType::ADD;
	case Op::Subtract: return InstructionType::SUBTRACT;
	case Op::Multiply: return InstructionType::MULTIPLY;
	case Op::Divide: return InstructionType::DIVIDE;
	case Op::Equals: return InstructionType::COMP_EQ;
	case Op::NotEquals: return InstructionType::COMP_NE;
	case Op::Greater: return InstructionType::COMP_GT;
	case Op::GreaterOrEqual: return InstructionType::COMP_GE;
	case Op::Less: return InstructionType::COMP_LT;
	case Op::LessOrEqual: return InstructionType::COMP_LE;
	case Op::UnaryMinus: return InstructionType::NEGATIVE;
	case Op::UnaryPlus: return InstructionType::POSITIVE;
	case Op::Function: return InstructionType::FUNCTION_OPERATOR;
	default:
		throw std::runtime_error("unexpected parser state");
	}
}

bool Carbon::Parser::ParseExpression()
{
	switch (auto token = lexer.GetToken())
	{
	case Token::Local:
		consumeToken = true; // consume function token
		state.pop();
		state.push(State::LocalEnd);
		state.push(State::Expression);
		break;
		
	case Token::Function:// function expression
		instruction = InstructionType::FUNCTIONBEGIN;
		consumeToken = true; // consume function token
		state.push(State::FunctionBegin);
		return false; // yield FUNCTIONBEGIN
		break;
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
		if (opStack.top() == Op::Term)
		{
			throw ParseError("expecting operator or end of expression");
		}
		instruction = TokenToAtom(token);
		instructionData = lexer.GetData();
		consumeToken = true; // consume token
		// check if unary on top of opstack
		if (OpIsUnary(opStack.top()))
		{
			// pop unary operators after yielding
			state.push(State::ExpressionPopUnary);
		}
		else
		{
			opStack.push(Op::Term);
		}
		return false; // yield
	case Token::ParanthesisOpen:
		if (opStack.top() == Op::Term)
		{
			opStack.pop();
			opStack.push(Op::FunctionCall);
			state.push(State::CallList);
			consumeToken = true; // consume token
			instruction = InstructionType::CALLBEGIN;
			return false; // yield callbegin
		}
		else
		{
			opStack.push(Op::Paranthesis);
			consumeToken = true; // consume token
			expressionPrevAtom = false;
			expressionPrevOp = false;
			return true; // continue parsing
		}
		return true;
		
	case Token::ParanthesisClose:
		// check for errorstate
		if (opStack.size() <= 0) {
			throw ParseError("mismatched paranthesis");
		}
		// check if term is on top
		if (opStack.top() == Op::Term)
		{
			// yep, term was the last token in expression, this is pretty normal
			opStack.pop();
		}
		// check for for ending function call
		if (opStack.top() == Op::FunctionCall)
		{
			consumeToken = true; // consume token
			opStack.pop();
			opStack.push(Op::Term);
			instruction = InstructionType::CALLEND;
			return false; // continue parsing
		}
		// check for opening paranthesis
		else if (opStack.top() == Op::Paranthesis) {
			consumeToken = true; // consume token
			opStack.pop();
			opStack.push(Op::Term);
			return true; // continue parsing
		}
		// end of subexpression
		else if (opStack.top() == Op::Expression)
		{
			state.pop();
			opStack.pop();
			return true;
		}
		// yield operators until we get to either Paranthesis or FunctionCall
		else {
			instruction = OpToInstructionType(opStack.top());
			opStack.pop();
			return false; // yield operator
		}
		return true;

	case Token::BracketOpen:
		// check if we had term last
		if (opStack.top() == Op::Term)
		{
			// a[3] array sbuscriut
			throw ParseError("Array subscript is not supported yet");
		}
		opStack.push(Op::Bracket);
		state.push(State::CallList);
		consumeToken = true; // consume token
		instruction = InstructionType::ARRAYBEGIN;
		return false; // yield instruction

	case Token::BracesOpen:
		// check if we had term last
		if (opStack.top() == Op::Term)
		{
			// "a{x:3}" is not valid syntax
			throw ParseError("Unexpected object, expecting operator or function call");
		}
		opStack.push(Op::Braces);
		state.push(State::KeyValue);
		consumeToken = true; // consume token
		instruction = InstructionType::OBJECTBEGIN;
		return false; // yield instruction

	// operators
	case Token::Plus:
	case Token::Minus:
		if (opStack.top() != Op::Term)
		{
			// yep we have a unary operator!
			opStack.push(TokenToUnaryOp(token));
			consumeToken = true; // consume token
			return true; //continue parsing
		}
		// nope, not a unary op, continue to binary ops
	case Token::Multiply:
	case Token::Divide:
	case Token::Equals:
	case Token::NotEquals:
	case Token::Greater:
	case Token::GreaterOrEqual:
	case Token::Less:
	case Token::LessOrEqual:
	case Token::FunctionOperator:
	case Token::Assign: {
		// expecting binary op
		if (opStack.top() != Op::Term) // it must have a valid left hand side
		{
			throw ParseError("unexpected operator");
		}
		// parse binary op
		bool push = true;
		Op currentOp = TokenToBinaryOp(token);
		// temporarely remove term, this is needed for precedence comparison as previous op is below the required term
		opStack.pop();
		if (opStack.size() > 0)
		{
			auto precedenceToken = OpPrecedence(currentOp);
			auto precedenceOpstackTop = OpPrecedence(opStack.top());
			if (precedenceToken < precedenceOpstackTop)
			{
				push = false;
			}
			else if (precedenceToken == precedenceOpstackTop && !OpIsRightAssociative(currentOp))
			{
				push = false;
			}
		}
		if (push)
		{
			consumeToken = true; // consume token
			// term is consumed don't add it back
			opStack.push(currentOp);
			return true; //continue parsing
		}
		else
		{
			instruction = OpToInstructionType(opStack.top());
			// consume operator on opstack
			opStack.pop();
			// addback term which was removed in the beginning
			opStack.push(Op::Term);
			return false;
		}
	}
		break;


	case Token::BracketClose:
		// should have baracket on top
		if (opStack.top() == Op::Bracket)
		{
			opStack.pop();
			opStack.push(Op::Term);
			consumeToken = true; // consume token
			instruction = InstructionType::ARRAYEND;
			return false; // yield instruction
		}
		// purposefully fall through, closing bracket needs to by handled by parent expression
	case Token::BracesClose:
		// we should have the opening braces on the opstack
		if (opStack.top() == Op::Braces)
		{
			opStack.pop(); // remove braces
			opStack.push(Op::Term);
			consumeToken = true; // consume token
			instruction = InstructionType::OBJECTEND;
			return false; // yield instruction
		}
		// purpusefully fall through, closing braces are ending current expression
	case Token::FileEnd:
	case Token::EndStatement:
	case Token::Comma:
		if (opStack.size()>0)
		{
			// check if term is on top
			if (opStack.top()==Op::Term)
			{
				// yep, term was the last token in expression, this is pretty normal
				opStack.pop();
			}
			// now lets check if we reached the end of expression
			if (opStack.top()==Op::Expression)
			{
				state.pop();
				opStack.pop();
				return true; // end of epxression, continue parsing
			}
			else
			{
				// no, we have some more instructions left
				instruction = OpToInstructionType(opStack.top());
				opStack.pop();
				return false; // yield
			}
		}
		else
		{
			throw ParseError("Invalid parser state");
		}
		break;
	default:
		throw ParseError();
		break;
	}
}

bool Carbon::Parser::ParseExpressionPopUnary()
{
	auto op = opStack.top();
	if (OpIsUnary(op))
	{
		instruction = OpToInstructionType(op);
		opStack.pop();
		return false; // yield
	}
	else
	{
		state.pop();
		opStack.push(Op::Term);
		return true; // continue parsing
	}
}

Carbon::InstructionType Carbon::Parser::TokenToAtom(Token token)
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

Carbon::Parser::Op Carbon::Parser::TokenToBinaryOp(Token token)
{
	switch (token)
	{
		case Token::Assign: 
			return Op::Assign;
		case Token::Plus:
			return Op::Add;
		case Token::Minus:
			return Op::Subtract;
		case Token::Multiply:
			return Op::Multiply;
		case Token::Divide:
			return Op::Divide;
		case Token::Equals: 
			return Op::Equals;
		case Token::NotEquals: 
			return Op::NotEquals;
		case Token::Greater:
			return Op::Greater;
		case Token::GreaterOrEqual:
			return Op::GreaterOrEqual;
		case Token::Less:
			return Op::Less;
		case Token::LessOrEqual:
			return Op::LessOrEqual;
		case Token::FunctionOperator:
			return Op::Function;
		default:
			throw ParseError("Invalid operator");
	}
}

Carbon::Parser::Op Carbon::Parser::TokenToUnaryOp(Token token)
{
	switch (token)
	{
	case Token::Plus:
		return Op::UnaryPlus;
	case Token::Minus:
		return Op::UnaryMinus;
	default:
		throw ParseError("Invalid operator");
	}
}

bool Carbon::Parser::OpIsUnary(Op op)
{
	switch (op)
	{
		case Op::UnaryMinus:
		case Op::UnaryPlus:
			return true;
		default:
			return false;
	}
}

Carbon::ParserException Carbon::Parser::ParseError()
{
	std::stringstream ss;
	ss << "Unexpected " << lexer.GetData();
	ParserException exception(ss.str());
	exception.Line = lexer.GetLine();
	exception.LinePosition = lexer.GetPosition();
	return exception;
}

Carbon::ParserException Carbon::Parser::ParseError(const char* message)
{
	ParserException exception(message);
	exception.Line = lexer.GetLine();
	exception.LinePosition = lexer.GetPosition();
	return exception;
}

