/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SCRIPTS_GENKSYMS_PARSE_TAB_H_INCLUDED
# define YY_YY_SCRIPTS_GENKSYMS_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ASM_KEYW = 258,                /* ASM_KEYW  */
    ATTRIBUTE_KEYW = 259,          /* ATTRIBUTE_KEYW  */
    AUTO_KEYW = 260,               /* AUTO_KEYW  */
    BOOL_KEYW = 261,               /* BOOL_KEYW  */
    BUILTIN_INT_KEYW = 262,        /* BUILTIN_INT_KEYW  */
    CHAR_KEYW = 263,               /* CHAR_KEYW  */
    CONST_KEYW = 264,              /* CONST_KEYW  */
    DOUBLE_KEYW = 265,             /* DOUBLE_KEYW  */
    ENUM_KEYW = 266,               /* ENUM_KEYW  */
    EXTERN_KEYW = 267,             /* EXTERN_KEYW  */
    EXTENSION_KEYW = 268,          /* EXTENSION_KEYW  */
    FLOAT_KEYW = 269,              /* FLOAT_KEYW  */
    INLINE_KEYW = 270,             /* INLINE_KEYW  */
    INT_KEYW = 271,                /* INT_KEYW  */
    LONG_KEYW = 272,               /* LONG_KEYW  */
    REGISTER_KEYW = 273,           /* REGISTER_KEYW  */
    RESTRICT_KEYW = 274,           /* RESTRICT_KEYW  */
    SHORT_KEYW = 275,              /* SHORT_KEYW  */
    SIGNED_KEYW = 276,             /* SIGNED_KEYW  */
    STATIC_KEYW = 277,             /* STATIC_KEYW  */
    STATIC_ASSERT_KEYW = 278,      /* STATIC_ASSERT_KEYW  */
    STRUCT_KEYW = 279,             /* STRUCT_KEYW  */
    TYPEDEF_KEYW = 280,            /* TYPEDEF_KEYW  */
    UNION_KEYW = 281,              /* UNION_KEYW  */
    UNSIGNED_KEYW = 282,           /* UNSIGNED_KEYW  */
    VOID_KEYW = 283,               /* VOID_KEYW  */
    VOLATILE_KEYW = 284,           /* VOLATILE_KEYW  */
    TYPEOF_KEYW = 285,             /* TYPEOF_KEYW  */
    VA_LIST_KEYW = 286,            /* VA_LIST_KEYW  */
    X86_SEG_KEYW = 287,            /* X86_SEG_KEYW  */
    EXPORT_SYMBOL_KEYW = 288,      /* EXPORT_SYMBOL_KEYW  */
    ASM_PHRASE = 289,              /* ASM_PHRASE  */
    ATTRIBUTE_PHRASE = 290,        /* ATTRIBUTE_PHRASE  */
    TYPEOF_PHRASE = 291,           /* TYPEOF_PHRASE  */
    BRACE_PHRASE = 292,            /* BRACE_PHRASE  */
    BRACKET_PHRASE = 293,          /* BRACKET_PHRASE  */
    EXPRESSION_PHRASE = 294,       /* EXPRESSION_PHRASE  */
    STATIC_ASSERT_PHRASE = 295,    /* STATIC_ASSERT_PHRASE  */
    CHAR = 296,                    /* CHAR  */
    DOTS = 297,                    /* DOTS  */
    IDENT = 298,                   /* IDENT  */
    INT = 299,                     /* INT  */
    REAL = 300,                    /* REAL  */
    STRING = 301,                  /* STRING  */
    TYPE = 302,                    /* TYPE  */
    OTHER = 303,                   /* OTHER  */
    FILENAME = 304                 /* FILENAME  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_SCRIPTS_GENKSYMS_PARSE_TAB_H_INCLUDED  */
