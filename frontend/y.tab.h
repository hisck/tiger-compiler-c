/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LOW = 258,
    TYPE = 259,
    FUNCTION = 260,
    ID = 261,
    LBRACK = 262,
    DO = 263,
    OF = 264,
    THEN = 265,
    ELSE = 266,
    SEMICOLON = 267,
    ASSIGN = 268,
    OR = 269,
    AND = 270,
    EQ = 271,
    NEQ = 272,
    LT = 273,
    LE = 274,
    GT = 275,
    GE = 276,
    PLUS = 277,
    MINUS = 278,
    TIMES = 279,
    DIVIDE = 280,
    UMINUS = 281,
    STRING = 282,
    INT = 283,
    COMMA = 284,
    COLON = 285,
    LPAREN = 286,
    RPAREN = 287,
    RBRACK = 288,
    LBRACE = 289,
    RBRACE = 290,
    DOT = 291,
    ARRAY = 292,
    IF = 293,
    WHILE = 294,
    FOR = 295,
    TO = 296,
    LET = 297,
    IN = 298,
    END = 299,
    BREAK = 300,
    NIL = 301,
    VAR = 302
  };
#endif
/* Tokens.  */
#define LOW 258
#define TYPE 259
#define FUNCTION 260
#define ID 261
#define LBRACK 262
#define DO 263
#define OF 264
#define THEN 265
#define ELSE 266
#define SEMICOLON 267
#define ASSIGN 268
#define OR 269
#define AND 270
#define EQ 271
#define NEQ 272
#define LT 273
#define LE 274
#define GT 275
#define GE 276
#define PLUS 277
#define MINUS 278
#define TIMES 279
#define DIVIDE 280
#define UMINUS 281
#define STRING 282
#define INT 283
#define COMMA 284
#define COLON 285
#define LPAREN 286
#define RPAREN 287
#define RBRACK 288
#define LBRACE 289
#define RBRACE 290
#define DOT 291
#define ARRAY 292
#define IF 293
#define WHILE 294
#define FOR 295
#define TO 296
#define LET 297
#define IN 298
#define END 299
#define BREAK 300
#define NIL 301
#define VAR 302

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "tiger.y"

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
  

#line 172 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
