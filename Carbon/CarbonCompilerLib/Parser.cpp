#include "Parser.h"

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
	default:
		throw std::runtime_error("Unexpected compiler state.");
		break;
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

bool Carbon::Parser::ParseStatement()
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
	case Token::Minus:
	case Token::Plus:
		state.push(State::Expression); // start parsing expression
		opStack.push(Op::Expression);
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

bool Carbon::Parser::ParseBlockOrObject()
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
	this->instructionData = tempBuffer.c_str();
	return false; //yield ATOM
}

bool Carbon::Parser::ParseBlock()
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
		lexer.MoveNext(); // consume token
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
		lexer.MoveNext(); // consume colon
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
		lexer.MoveNext(); // consume comma
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
		lexer.MoveNext(); // consume token
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
		lexer.MoveNext(); // consume id
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
		lexer.MoveNext(); // consume comma
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
		lexer.MoveNext(); // consume parathesis
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
		lexer.MoveNext(); // consume parathesis
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
	default:
		throw std::runtime_error("unexpected parser state");
	}
}

bool Carbon::Parser::ParseExpression()
{
	switch (auto token = lexer.GetToken())
	{
	case Token::Local:
		lexer.MoveNext(); // consume function token
		state.pop();
		state.push(State::LocalEnd);
		state.push(State::Expression);
		break;
		
	case Token::Function:// function expression
		instruction = InstructionType::FUNCTIONBEGIN;
		lexer.MoveNext(); // consume function token
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
		lexer.MoveNext(); // consume token
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
			lexer.MoveNext(); // consume token
			instruction = InstructionType::CALLBEGIN;
			return false; // yield callbegin
		}
		else
		{
			opStack.push(Op::Paranthesis);
			lexer.MoveNext(); // consume token
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
			lexer.MoveNext(); // consume token
			opStack.pop();
			opStack.push(Op::Term);
			instruction = InstructionType::CALLEND;
			return false; // continue parsing
		}
		// check for opening paranthesis
		else if (opStack.top() == Op::Paranthesis) {
			lexer.MoveNext(); // consume token
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
		lexer.MoveNext(); // consume token
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
		lexer.MoveNext(); // consume token
		instruction = InstructionType::OBJECTBEGIN;
		return false; // yield instruction

	// operators
	case Token::Plus:
	case Token::Minus:
		if (opStack.top() != Op::Term)
		{
			// yep we have a unary operator!
			opStack.push(TokenToUnaryOp(token));
			lexer.MoveNext(); // consume token
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
			lexer.MoveNext(); // consume token
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
			lexer.MoveNext(); // consume token
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
			lexer.MoveNext(); // consume token
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
			throw ParseError("invalid parser state");
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
		default:
			throw ParseError("invalid operator");
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
		throw ParseError("invalid operator");
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

std::invalid_argument Carbon::Parser::ParseError()
{
	std::stringstream ss;
	ss << "Unexpected" << lexer.GetData() << " at line " << lexer.GetLine() << " position " << lexer.GetPosition() << "\n";
	return std::invalid_argument(ss.str());
}

std::invalid_argument Carbon::Parser::ParseError(const char* message)
{
	std::stringstream ss;
	ss << message << " at line " << lexer.GetLine() << " position " << lexer.GetPosition() << "\n";
	return std::invalid_argument(ss.str());
}

