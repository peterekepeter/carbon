#pragma once

namespace Carbon
{
	namespace Compiler
	{
		// all token types generated by the lexer
		enum class Token
		{
			// most important
			Id,
			FileEnd,
			FileBegin,

			// string datatypes
			StringUnicode,
			StringBinary,
			String,

			// numeric datatypes
			Float,
			NumberHexadecimal,
			NumberBinary,
			NumberOctal,
			Number,

			// syntax symbols

			ParanthesisOpen,
			ParanthesisClose,
			BracketOpen,
			BracketClose,
			BracesOpen,
			BracesClose,
			EndStatement,

			// operators

			Equals,
			NotEquals,
			LessOrEqual,
			GreaterOrEqual,
			Assign,
			Less,
			Greater,
			Plus,
			Minus,
			Multiply,
			Divide,
			Not,
			Member,

			// keywords

			Function,
			Loop,
			Else,
			If,
			Then,
			Local,
			Break,
			Continue,
			Return,
		};

	}
}