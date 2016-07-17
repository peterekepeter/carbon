%option outfile="lexer.cpp"
%option noyywrap
%option yylineno


%{
#include "parser.hpp"

%}

%%

"function" { return TOK_KEY_FUNCTION; }
"local" { return TOK_KEY_LOCAL; }
"loop" { return TOK_KEY_LOOP; }
"else" { return TOK_KEY_ELSE; }
"break" { return TOK_KEY_BREAK; }
"continue" { return TOK_KEY_CONTINUE; }
"return" { return TOK_KEY_RETURN; }
"if" { return TOK_KEY_IF; }

[lL]\"(\\.|[^"])*\"     { return TOK_USTR; }
[xX]\"(\\.|[^"])*\"     { return TOK_XSTR; }
[bB]\"(\\.|[^"])*\"     { return TOK_BSTR; }
\"(\\.|[^"])*\"         { return TOK_STR; }
[_a-zA-Z][_0-9a-zA-Z]*  { return TOK_ID; }
([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)(e[+-]?[0-9]+)? { return TOK_FLOAT; }
0[xX][0-9a-fA-F]+       { return TOK_XNUM; }
0[bB][0-9a-fA-F]+       { return TOK_BNUM; }
0[0-9a-fA-F]+           { return TOK_ONUM; }
[0-9][0-9a-fA-F]*       { return TOK_NUM; }

"==" { return TOK_OP_EQ; }
"!=" { return TOK_OP_NE; }
"<=" { return TOK_OP_LE; }
">=" { return TOK_OP_GE; }

; { return TOK_ENDST; }
[ \t\n]+                { ; }
.                       { return yytext[0]; }
%%

