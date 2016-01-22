


%{
#include "types.hpp"
#include "lexer.hpp"
#include "yyerror.hpp"
#include "exec.hpp"
using namespace exec;
extern Executor ex;
%}

%token USTR
%token XSTR
%token BSTR
%token STR
%token ID
%token FLOAT
%token XNUM
%token BNUM
%token ONUM
%token NUM
%token ENDST
%token OP_EQ
%token OP_NE
%token OP_LE
%token OP_GE
%token KEY_FUNCTION
%token KEY_LOOP
%token KEY_ELSE
%token KEY_IF
%token KEY_THEN
%token KEY_LOCAL
%token KEY_BREAK
%token KEY_CONTINUE
%token KEY_RETURN

%right '='
%left OP_EQ OP_NE
%left '<' '>' OP_LE OP_GE
%left '+' '-'
%left '*' '/'

%%

program : //nothing    
        | program statement 
        ;

statement : '{' {ex.submitCommand(COMMANDTYPE::BLOCKBEGIN);} program '}' {ex.submitCommand(COMMANDTYPE::BLOCKEND);}
          | KEY_LOOP {ex.submitCommand(COMMANDTYPE::CONTROL);} loop 
          | KEY_IF {ex.submitCommand(COMMANDTYPE::CONTROL);} '(' expression ')' statement else
          | expression ENDST  { ex.submitCommand(COMMANDTYPE::END_STATEMENT); }
          | KEY_RETURN ENDST { ex.submitCommand(COMMANDTYPE::RETURN0); }
          | KEY_RETURN expression ENDST { ex.submitCommand(COMMANDTYPE::RETURN1); }
          | KEY_BREAK ENDST { ex.submitCommand(COMMANDTYPE::BREAK); }
          | KEY_CONTINUE ENDST { ex.submitCommand(COMMANDTYPE::CONTINUE); }
          | ENDST
          ;

else: KEY_ELSE statement {ex.submitCommand(COMMANDTYPE::IFELSE);} 
    | {ex.submitCommand(COMMANDTYPE::IF);}
    ;

loop: statement {ex.submitCommand(COMMANDTYPE::LOOP0);}
    | '(' expression ')' statement {ex.submitCommand(COMMANDTYPE::LOOP1);}
    | '(' expression ',' expression ')' statement {ex.submitCommand(COMMANDTYPE::LOOP2);} 
    | '(' expression ',' expression ',' expression ')' statement {ex.submitCommand(COMMANDTYPE::LOOP3);} 
    ;


expression: KEY_FUNCTION '(' { ex.submitCommand(COMMANDTYPE::FUNCTIONBEGIN); } idlist ')' statement { ex.submitCommand(COMMANDTYPE::FUNCTIONEND); }
          | KEY_LOCAL expression { ex.submitCommand(COMMANDTYPE::LOCAL); }
          | atom '(' { ex.submitCommand(COMMANDTYPE::CALLBEGIN); } calllist ')' { ex.submitCommand(COMMANDTYPE::CALLEND); }
          | atom '=' expression       { ex.submitCommand(COMMANDTYPE::ASSIGN); }
          | expression OP_EQ expression { ex.submitCommand(COMMANDTYPE::COMP_EQ);} 
          | expression OP_NE expression { ex.submitCommand(COMMANDTYPE::COMP_NE);} 
          | expression OP_LE expression { ex.submitCommand(COMMANDTYPE::COMP_LE);} 
          | expression OP_GE expression { ex.submitCommand(COMMANDTYPE::COMP_GE);} 
          | expression '<' expression { ex.submitCommand(COMMANDTYPE::COMP_LT);} 
          | expression '>' expression { ex.submitCommand(COMMANDTYPE::COMP_GT);} 
          | expression '+' expression { ex.submitCommand(COMMANDTYPE::ADD);} 
          | expression '-' expression { ex.submitCommand(COMMANDTYPE::SUBTRACT); }
          | expression '*' expression { ex.submitCommand(COMMANDTYPE::MULTIPLY); }
          | expression '/' expression { ex.submitCommand(COMMANDTYPE::DIVIDE); }
          | '+' expression            { ex.submitCommand(COMMANDTYPE::POSITIVE); }
          | '-' expression            { ex.submitCommand(COMMANDTYPE::NEGATIVE); }
          | '(' expression ')'
          | atom 
          ;

idlist: //nothing
      | idlist ',' identifier
      | identifier
      ;

identifier: ID { ex.submitAtom(ID, yytext); }
          ;

calllist: //nothing
        | calllist ',' expression
        | expression
        ;

atom: USTR          { ex.submitAtom(USTR, yytext); }
    | XSTR          { ex.submitAtom(XSTR, yytext); }
    | BSTR          { ex.submitAtom(BSTR, yytext); }
    | STR           { ex.submitAtom(STR, yytext); }
    | ID            { ex.submitAtom(ID, yytext); }
    | FLOAT         { ex.submitAtom(FLOAT, yytext); }
    | XNUM          { ex.submitAtom(XNUM, yytext); }
    | BNUM          { ex.submitAtom(BNUM, yytext); }
    | ONUM          { ex.submitAtom(ONUM, yytext); }
    | NUM           { ex.submitAtom(NUM, yytext); }
    ;

%%