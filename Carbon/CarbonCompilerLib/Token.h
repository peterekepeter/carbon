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
			Colon,
			Comma,

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


		// all token types generated by the lexer
		constexpr int TokenPrecedence(Token token) noexcept
		{
			switch (token)
			{
			case Token::Assign:
				return 4;
			case Token::Equals:
			case Token::NotEquals:
				return 5;
			case Token::GreaterOrEqual:
			case Token::Greater:
			case Token::Less:
			case Token::LessOrEqual:
				return 6;
			case Token::Plus:
			case Token::Minus:
				return 7;
			case Token::Multiply:
			case Token::Divide:
				return 8;
			default: 
				return 0;
			}
		}

		// returns if token is right associative, otherwise false
		constexpr bool TokenIsRightAssociative(Token token) noexcept
		{
			switch (token)
			{
			case Token::Assign:
				return true;
			default:
				return false;
			}
		}

	}
}