#pragma once

namespace Carbon
{
	// forward declaration
	enum class InstructionType : unsigned char;
	
	// check if instruction needs string data
	constexpr bool InstructionNeedsString(InstructionType itype);

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
		ARRAY, //internal
		ARRAYBEGIN,
		ARRAYEND,
		OBJECT, //internal
		OBJECTBEGIN,
		OBJECTEND,
		COMP_LT,
		COMP_GT,
		COMP_LE,
		COMP_GE,
		COMP_EQ,
		COMP_NE,
		MEMBER,
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
		FUNCTION_OPERATOR,

		// it is assumed that all types ater ID require string data
		ID,   
		USTR, 
		XSTR, 
		BSTR, 
		STR,  
		FLOAT,
		XNUM, 
		BNUM, 
		ONUM, 
		NUM   
	};

	constexpr bool InstructionNeedsString(InstructionType itype)
	{
		return itype >= InstructionType::ID;
	}
}