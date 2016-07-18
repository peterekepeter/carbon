


%{
#include "lexer.hpp"
#include "yyerror.hpp"
#include "InstructionWriter.hpp"
extern InstructionWriter* yyInstructionWriter;
#define INS_CMD(A)    yyInstructionWriter->WriteInstruction(InstructionType::A)
#define INS_ATOM(A) yyInstructionWriter->WriteInstruction(InstructionType::A, yytext)
%}

%token TOK_USTR
%token TOK_XSTR
%token TOK_BSTR
%token TOK_STR
%token TOK_ID
%token TOK_FLOAT
%token TOK_XNUM
%token TOK_BNUM
%token TOK_ONUM
%token TOK_NUM
%token TOK_ENDST
%token TOK_OP_EQ
%token TOK_OP_NE
%token TOK_OP_LE
%token TOK_OP_GE
%token TOK_KEY_FUNCTION
%token TOK_KEY_LOOP
%token TOK_KEY_ELSE
%token TOK_KEY_IF
%token TOK_KEY_THEN
%token TOK_KEY_LOCAL
%token TOK_KEY_BREAK
%token TOK_KEY_CONTINUE
%token TOK_KEY_RETURN

%right '='
%left TOK_OP_EQ TOK_OP_NE
%left '<' '>' TOK_OP_LE TOK_OP_GE
%left '+' '-'
%left '*' '/'
%left '.'

%%

program : //nothing    
        | program statement 
        ;

statement : '{' {INS_CMD(BLOCKBEGIN);} program '}' {INS_CMD(BLOCKEND);}
          | TOK_KEY_LOOP {INS_CMD(CONTROL);} loop 
          | TOK_KEY_IF {INS_CMD(CONTROL);} '(' expression ')' statement else
          | expression TOK_ENDST  { INS_CMD(END_STATEMENT); }
          | TOK_KEY_RETURN TOK_ENDST { INS_CMD(RETURN0); }
          | TOK_KEY_RETURN expression TOK_ENDST { INS_CMD(RETURN1); }
          | TOK_KEY_BREAK TOK_ENDST { INS_CMD(BREAK); }
          | TOK_KEY_CONTINUE TOK_ENDST { INS_CMD(CONTINUE); }
          | TOK_ENDST
          ;

else: TOK_KEY_ELSE statement {INS_CMD(IFELSE);} 
    | {INS_CMD(IF);}
    ;

loop: statement {INS_CMD(LOOP0);}
    | '(' expression ')' statement {INS_CMD(LOOP1);}
    | '(' expression ',' expression ')' statement {INS_CMD(LOOP2);} 
    | '(' expression ',' expression ',' expression ')' statement {INS_CMD(LOOP3);} 
    ;


expression: TOK_KEY_FUNCTION '(' { INS_CMD(FUNCTIONBEGIN); } idlist ')' statement { INS_CMD(FUNCTIONEND); }
          | TOK_KEY_LOCAL expression { INS_CMD(LOCAL); }
          | atom '(' { INS_CMD(CALLBEGIN); } calllist ')' { INS_CMD(CALLEND); }
          | atom '=' expression       { INS_CMD(ASSIGN); }
          | expression TOK_OP_EQ expression { INS_CMD(COMP_EQ);} 
          | expression TOK_OP_NE expression { INS_CMD(COMP_NE);} 
          | expression TOK_OP_LE expression { INS_CMD(COMP_LE);} 
          | expression TOK_OP_GE expression { INS_CMD(COMP_GE);} 
          | expression '<' expression { INS_CMD(COMP_LT);} 
          | expression '>' expression { INS_CMD(COMP_GT);} 
          | expression '+' expression { INS_CMD(ADD);} 
          | expression '-' expression { INS_CMD(SUBTRACT); }
          | expression '*' expression { INS_CMD(MULTIPLY); }
          | expression '/' expression { INS_CMD(DIVIDE); }
		  | expression '.' expression { INS_CMD(MEMBER); }
          | '+' expression            { INS_CMD(POSITIVE); }
          | '-' expression            { INS_CMD(NEGATIVE); }
          | '(' expression ')'
          | atom 
          ;

idlist: //nothing
      | idlist ',' identifier
      | identifier
      ;

identifier: TOK_ID { INS_ATOM(ID); }
          ;

calllist: //nothing
        | calllist ',' expression
        | expression
        ;

atom: TOK_USTR          { INS_ATOM(USTR); }
    | TOK_XSTR          { INS_ATOM(XSTR); }
    | TOK_BSTR          { INS_ATOM(BSTR); }
    | TOK_STR           { INS_ATOM(STR); }
    | TOK_ID            { INS_ATOM(ID); }
    | TOK_FLOAT         { INS_ATOM(FLOAT); }
    | TOK_XNUM          { INS_ATOM(XNUM); }
    | TOK_BNUM          { INS_ATOM(BNUM); }
    | TOK_ONUM          { INS_ATOM(ONUM); }
    | TOK_NUM           { INS_ATOM(NUM); }
    ;

%%