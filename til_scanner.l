%option c++ prefix="til_scanner_" outfile="til_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "til_parser.tab.h"

// don't change this
#define yyerror LexerError
%}

EXP [eE][+-]?[0-9]+

%x X_COMMENT
%x X_STRING X_BACKLASH X_NULL 
%x X_HEX_INT

%%
  yydebug=1; set_debug(1);

  /* ====================================================================== */
  /* ====[                     3.1 - white spaces                     ]==== */
  /* ====================================================================== */

[ \t\r\n]+ ; 

  /* ====================================================================== */
  /* ====[                      3.2 - comments                        ]==== */
  /* ====================================================================== */

";".*$            ; 
"/*"              yy_push_state(X_COMMENT);
<X_COMMENT>"/*"   yy_push_state(X_COMMENT); // nested comment
<X_COMMENT>"*/"   yy_pop_state();
<X_COMMENT>.|"\n" ;

  /* ====================================================================== */
  /* ====[                      3.3 - keywords                        ]==== */
  /* ====================================================================== */

">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE; 
"&&"                   return tAND;
"||"                   return tOR;

"external"             return tEXTERNAL;
"forward"              return tFORWARD;
"public"               return tPUBLIC;
"var"                  return tVAR;

"program"              return tPROGRAM;
"function"             return tFUNCTION;
"if"                   return tIF; 
"block"                return tBLOCK;
"loop"                 return tLOOP; 
"next"                 return tNEXT;
"stop"                 return tSTOP;
"return"               return tRETURN;

"read"                 return tREAD; 
"print"                return tPRINT; 
"println"              return tPRINTLN; 

"set"                  return tSET;
"null"                 return tNULL;

"sizeof"               return tSIZEOF;
"objects"              return tOBJECTS;
"index"                return tINDEX;

  /* ====================================================================== */
  /* ====[                        3.4 - types                         ]==== */
  /* ====================================================================== */

"int"                  return tTYPE_INT;
"double"               return tTYPE_DOUBLE;
"string"               return tTYPE_STRING;
"void"                 return tTYPE_VOID;

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

  /* ====================================================================== */
  /* ====[                       3.8 - literals                       ]==== */
  /* ====================================================================== */
  /* ====================================================================== */
  /* ====[                     3.8.1 - integers                       ]==== */
  /* ====================================================================== */

0|[1-9][0-9]*   { // int decimal notation
                  try {
                    yylval.i = std::stoi(yytext, nullptr, 10);
                    return tINTEGER;
                  } catch (std::out_of_range &) {
                    yyerror("integer overflow");
                  }
                }

0x[0-9a-fA-F]+  { // int hexadecimal notation
                  try {
                    yylval.i = std::stoi(yytext, nullptr, 16);
                    return tINTEGER;
                  } catch (std::out_of_range &) {
                    yyerror("integer overflow");
                  }
                }

0x[0-9a-zA-Z]+  yyerror("invalid hexadecimal notation");

  /* ====================================================================== */
  /* ====[              3.8.2 - floating point number                 ]==== */
  /* ====================================================================== */

[0-9]*\.[0-9]+{EXP}? |
[0-9]+\.[0-9]*{EXP}? | 
[0-9]+{EXP}           { // double decimal notation
                        try {
                          yylval.d = std::stod(yytext, nullptr);
                          return tDOUBLE;
                        } catch (std::out_of_range &) {
                          yyerror("double overflow");
                        }
                      }

  /* ====================================================================== */
  /* ====[                      3.8.3 - strings                       ]==== */
  /* ====================================================================== */

\"                     yy_push_state(X_STRING); yylval.s = new std::string("");
<X_STRING>\"           yy_pop_state(); return tSTRING;
<X_STRING>\\           yy_push_state(X_BACKLASH);
<X_STRING>\n           yyerror("newline in string");
<X_STRING>\0           yyerror("null byte in string");
<X_STRING>.            *yylval.s += yytext;

<X_BACKLASH>n               yy_pop_state(); *yylval.s += '\n';
<X_BACKLASH>t               yy_pop_state(); *yylval.s += '\t';
<X_BACKLASH>r               yy_pop_state(); *yylval.s += '\r';
<X_BACKLASH>\"              yy_pop_state(); *yylval.s += '\"';
<X_BACKLASH>\\              yy_pop_state(); *yylval.s += '\\';
<X_BACKLASH>0               yy_push_state(X_NULL);
<X_BACKLASH>[0-3][0-7]{0,2} yy_pop_state(); *yylval.s += strtoul(yytext, NULL, 8); 
<X_BACKLASH>[4-7][0-7]{2}   yyerror("special character overflow"); // octal sequence out of 8bit repr range
<X_BACKLASH>.               yy_pop_state(); *yylval.s += yytext;

  /* ignore everything but fail if ASCII LF or ASCII NUL are present */
<X_NULL>\"             yy_pop_state(); yy_pop_state(); return tSTRING;
<X_NULL>\n             yyerror("newline in string");
<X_NULL>\0             yyerror("null byte in string");
<X_NULL>.|\\\"|\\\\    ;

[0-9]+                 yylval.i = strtol(yytext, nullptr, 10); return tINTEGER;

[-<>=+*/%;{}.@?~()!]    return *yytext;

.                      yyerror("Unknown character");

%%
