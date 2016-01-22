%option outfile="lexer.cpp"
%option noyywrap
%option yylineno


%{
#include "parser.hpp"

%}

%%

"function" { return KEY_FUNCTION; }
"local" { return KEY_LOCAL; }
"loop" { return KEY_LOOP; }
"else" { return KEY_ELSE; }
"break" { return KEY_BREAK; }
"continue" { return KEY_CONTINUE; }
"return" { return KEY_RETURN; }
"if" { return KEY_IF; }

[lL]\"(\\.|[^"])*\"     { return USTR; }
[xX]\"(\\.|[^"])*\"     { return XSTR; }
[bB]\"(\\.|[^"])*\"     { return BSTR; }
\"(\\.|[^"])*\"         { return STR; }
[_a-zA-Z][_0-9a-zA-Z]*  { return ID; }
([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)(e[+-]?[0-9]+)? { return FLOAT; }
0[xX][0-9a-fA-F]+       { return XNUM; }
0[bB][0-9a-fA-F]+       { return BNUM; }
0[0-9a-fA-F]+           { return ONUM; }
[0-9][0-9a-fA-F]*       { return NUM; }

"==" { return OP_EQ; }
"!=" { return OP_NE; }
"<=" { return OP_LE; }
">=" { return OP_GE; }

; { return ENDST; }
[ \t\n]+                { ; }
.                       { return yytext[0]; }
%%

