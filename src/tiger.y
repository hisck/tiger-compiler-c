%{
#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "absyn.h"
#include "symbol.h"

int yylex(void); /* function prototype */

void yyerror(char *s) {
  EM_error(EM_tokPos, "%s", s);
}

A_exp absyn_root;

%}

%union {
	int pos;
	int ival;
	string sval;
  A_var var;
  A_exp exp;
  A_dec dec;
  A_ty ty;
  A_decList declist;
  A_expList explist;
  A_field field;
  A_fieldList fieldlist;
  A_fundec fundec;
  A_fundecList fundeclist;
  A_namety namety;
  A_nametyList nametylist;
  A_efield efield;
  A_efieldList efieldlist;
  }

%nonassoc LOW
%nonassoc TYPE FUNCTION
%nonassoc ID
%nonassoc LBRACK
%nonassoc DO OF
%nonassoc THEN
%nonassoc ELSE
%left SEMICOLON
%nonassoc ASSIGN
%left OR
%left AND
%nonassoc EQ NEQ LT LE GT GE
%left PLUS MINUS
%left TIMES DIVIDE
%left UMINUS

%token <sval> ID STRING
%token <ival> INT
%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK 
  LBRACE RBRACE DOT 
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF 
  BREAK NIL
  FUNCTION VAR TYPE

%type <var> lvalue
%type <exp> program exp func_call arith_exp cmp_exp bool_exp record_create array_create
%type <dec> dec tydeclist vardec fundeclist
%type <ty> ty
%type <declist> decs
%type <explist> explist explist_nonempty expseq
%type <field> tyfield
%type <fieldlist> tyfields tyfields_nonempty
%type <fundec> fundec
%type <namety> tydec
%type <efield> record_create_field
%type <efieldlist> record_create_list record_create_list_nonempty

%start program

%%

program: exp { absyn_root = $1; } | {absyn_root = NULL ;}

exp: lvalue { $$ = A_VarExp(EM_tokPos, $1); }
   | NIL    { $$ = A_NilExp(EM_tokPos); }
   | INT           { $$ = A_IntExp(EM_tokPos, $1); }
   | STRING        { $$ = A_StringExp(EM_tokPos, $1); }
   | MINUS exp %prec UMINUS { $$ = A_OpExp(EM_tokPos, A_minusOp, A_IntExp(EM_tokPos, 0), $2); }
   | func_call     { $$ = $1; }
   | arith_exp     { $$ = $1; }
   | cmp_exp       { $$ = $1; }
   | bool_exp      { $$ = $1; }
   | record_create { $$ = $1; }
   | array_create  { $$ = $1; }
   | lvalue ASSIGN exp { $$ = A_AssignExp(EM_tokPos, $1, $3); }
   | IF exp THEN exp ELSE exp { $$ = A_IfExp(EM_tokPos, $2, $4, $6); }
   | IF exp THEN exp { $$ = A_IfExp(EM_tokPos, $2, $4, NULL); }
   | WHILE exp DO exp { $$ = A_WhileExp(EM_tokPos, $2, $4); }
   | DO LBRACE exp RBRACE WHILE LPAREN exp RPAREN { $$ = A_DoWhileExp(EM_tokPos, $3, $7); }
   | FOR ID ASSIGN exp TO exp DO exp { $$ = A_ForExp(EM_tokPos, S_Symbol($2), $4, $6, $8); }
   | BREAK { $$ = A_BreakExp(EM_tokPos); }
   | LET decs IN expseq END { $$ = A_LetExp(EM_tokPos, $2, A_SeqExp(EM_tokPos, $4)); }
   | LPAREN expseq RPAREN { $$ = A_SeqExp(EM_tokPos, $2); }

lvalue: ID                       { $$ = A_SimpleVar(EM_tokPos, S_Symbol($1)); }
      | ID LBRACK exp RBRACK     { $$ = A_SubscriptVar(EM_tokPos, A_SimpleVar(EM_tokPos, S_Symbol($1)), $3); }
      | lvalue LBRACK exp RBRACK { $$ = A_SubscriptVar(EM_tokPos, $1, $3); }
      | lvalue DOT ID            { $$ = A_FieldVar(EM_tokPos, $1, S_Symbol($3)); }

func_call: ID LPAREN explist RPAREN { $$ = A_CallExp(EM_tokPos, S_Symbol($1), $3); }

explist:                  { $$ = NULL; }
       | explist_nonempty { $$ = $1; }

explist_nonempty: exp                        { $$ = A_ExpList($1, NULL); }
                | exp COMMA explist_nonempty { $$ = A_ExpList($1, $3); }

arith_exp: exp PLUS exp   { $$ = A_OpExp(EM_tokPos, A_plusOp, $1, $3); }
         | exp MINUS exp  { $$ = A_OpExp(EM_tokPos, A_minusOp, $1, $3); }
         | exp TIMES exp  { $$ = A_OpExp(EM_tokPos, A_timesOp, $1, $3); }
         | exp DIVIDE exp { $$ = A_OpExp(EM_tokPos, A_divideOp, $1, $3); }

cmp_exp: exp EQ exp  { $$ = A_OpExp(EM_tokPos, A_eqOp, $1, $3); }
       | exp NEQ exp { $$ = A_OpExp(EM_tokPos, A_neqOp, $1, $3); }
       | exp LT exp  { $$ = A_OpExp(EM_tokPos, A_ltOp, $1, $3); }
       | exp LE exp  { $$ = A_OpExp(EM_tokPos, A_leOp, $1, $3); }
       | exp GT exp  { $$ = A_OpExp(EM_tokPos, A_gtOp, $1, $3); }
       | exp GE exp  { $$ = A_OpExp(EM_tokPos, A_geOp, $1, $3); }

bool_exp: exp AND exp { $$ = A_IfExp(EM_tokPos, $1, $3, A_IntExp(EM_tokPos, 0)); }
        | exp OR exp  { $$ = A_IfExp(EM_tokPos, $1, A_IntExp(EM_tokPos, 1), $3); }

record_create: ID LBRACE record_create_list RBRACE { $$ = A_RecordExp(EM_tokPos, S_Symbol($1), $3); }

record_create_list: { $$ = NULL; }
                  | record_create_list_nonempty { $$ = $1; }

record_create_list_nonempty: record_create_field { $$ = A_EfieldList($1, NULL); }
                           | record_create_field COMMA record_create_list_nonempty { $$ = A_EfieldList($1, $3); }

record_create_field: ID EQ exp { $$ = A_Efield(S_Symbol($1), $3); }

array_create: ID LBRACK exp RBRACK OF exp { $$ = A_ArrayExp(EM_tokPos, S_Symbol($1), $3, $6); }

decs:          { $$ = NULL; }
    | dec decs { $$ = A_DecList($1, $2); }

dec: tydeclist  { $$ = $1; }
   | vardec     { $$ = $1; }
   | fundeclist { $$ = $1; }

tydeclist: tydec %prec LOW { $$ = A_TypeDec(EM_tokPos, A_NametyList($1, NULL)); }
         | tydec tydeclist { $$ = A_TypeDec(EM_tokPos, A_NametyList($1, $2->u.type)); }

tydec: TYPE ID EQ ty       { $$ = A_Namety(S_Symbol($2), $4); }

ty: ID                     { $$ = A_NameTy(EM_tokPos, S_Symbol($1)); }
  | LBRACE tyfields RBRACE { $$ = A_RecordTy(EM_tokPos, $2); }
  | ARRAY OF ID            { $$ = A_ArrayTy(EM_tokPos, S_Symbol($3)); }

tyfields:                   { $$ = NULL; }
        | tyfields_nonempty { $$ = $1; }

tyfields_nonempty: tyfield                         { $$ = A_FieldList($1, NULL); }
                 | tyfield COMMA tyfields_nonempty { $$ = A_FieldList($1, $3); }

tyfield: ID COLON ID { $$ = A_Field(EM_tokPos, S_Symbol($1), S_Symbol($3)); }

vardec: VAR ID ASSIGN exp          { $$ = A_VarDec(EM_tokPos, S_Symbol($2), NULL, $4); }
      | VAR ID COLON ID ASSIGN exp { $$ = A_VarDec(EM_tokPos, S_Symbol($2), S_Symbol($4), $6); }

fundeclist: fundec %prec LOW  { $$ = A_FunctionDec(EM_tokPos, A_FundecList($1, NULL)); }
          | fundec fundeclist { $$ = A_FunctionDec(EM_tokPos, A_FundecList($1, $2->u.function)); }

fundec: FUNCTION ID LPAREN tyfields RPAREN EQ exp          { $$ = A_Fundec(EM_tokPos, S_Symbol($2), $4, NULL, $7); }
      | FUNCTION ID LPAREN tyfields RPAREN COLON ID EQ exp { $$ = A_Fundec(EM_tokPos, S_Symbol($2), $4, S_Symbol($7), $9); }

expseq: {$$=NULL;} 
      | exp                  { $$ = A_ExpList($1, NULL); }
      | exp SEMICOLON expseq { $$ = A_ExpList($1, $3); }