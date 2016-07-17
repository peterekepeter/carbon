#pragma once

enum class InstructionType : unsigned char {
	END_STATEMENT,
	ASSIGN,
	CALL, //internal
	CALLBEGIN,
	CALLEND,
	ADD,
	SUBTRACT,
	MULTIPLY,
	DIVIDE,
	POSITIVE,
	NEGATIVE,
	BLOCK, //internal
	BLOCKBEGIN,
	BLOCKEND,
	COMP_LT,
	COMP_GT,
	COMP_LE,
	COMP_GE,
	COMP_EQ,
	COMP_NE,
	CONTROL,
	LOCAL,
	IF,
	IFELSE,
	LOOP0,
	LOOP1,
	LOOP2,
	LOOP3,
	RETURN0,
	RETURN1,
	BREAK,
	CONTINUE,
	FUNCTIONBEGIN,
	FUNCTIONEND,
	USTR, // needs string data
	XSTR, // needs string data
	BSTR, // needs string data
	STR, // needs string data
	ID, // needs string data
	FLOAT, // needs string data
	XNUM, // needs string data
	BNUM, // needs string data
	ONUM, // needs string data
	NUM // needs string data
};
