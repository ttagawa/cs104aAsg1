/* $Id: lexer.l,v 1.3 2015-07-09 14:08:58-07 - - $ */

%{
#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { lexer_useraction(); }


%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER          [A-Za-z_]
DIGIT           [0-9]
MANTISSA        ({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
NUMBER          ({DIGIT}+)
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
NOTIDENT        ({DIGIT}{LETTER}+)
NOTCHAR         (’([^\\’\n]|\\[\\’"0nt]))
NOTSTRING       (\"([^\\"\n]|\\[\\’"0nt])*)
CHARCONS        (’([^\\’\n]|\\[\\’"0nt])’)
STRINGCONS      (\"([^\\"\n]|\\[\\’"0nt])*\")


%%

"#".*           { lexer_include(); }
[ \t]+          { }
\n              { lexer_newline(); }

{NUMBER}        { return yylval_token (TOK_INTCON); }
"void"          { return yylval_token (TOK_VOID); }
"bool"          { return yylval_token (TOK_BOOL); }
"char"          { return yylval_token (TOK_CHAR); }
"int"           { return yylval_token (TOK_INT); }
"string"        { return yylval_token (TOK_STRING); }
"struct"        { return yylval_token (TOK_STRUCT); }
"if"            { return yylval_token (TOK_IF); }
"else"          { return yylval_token (TOK_ELSE); }
"while"         { return yylval_token (TOK_WHILE); }
"return"        { return yylval_token (TOK_RETURN); }
"false"         { return yylval_token (TOK_FALSE); }
"true"          { return yylval_token (TOK_TRUE); }
"null"          { return yylval_token (TOK_NULL); }
"ord"           { return yylval_token (TOK_ORD); }
"chr"           { return yylval_token (TOK_CHR); }
"new"           { return yylval_token (TOK_NEW); }
{CHARCONS}      { return yylval_token (TOK_CHARCON); }
{STRINGCONS}    { return yylval_token (TOK_STRINGCON); }
{IDENT}         { return yylval_token (TOK_IDENT); }
"[]"            { return yylval_token (TOK_NEWARRAY); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
";"             { return yylval_token (';'); }
","             { return yylval_token (','); }
"."             { return yylval_token ('.'); }
"="             { return yylval_token ('='); }
"=="            { return yylval_token (TOK_EQ); }
"!="            { return yylval_token (TOK_NE); }
"<"             { return yylval_token (TOK_LT); }
"<="            { return yylval_token (TOK_LE); }
">"             { return yylval_token (TOK_GT); }
">="            { return yylval_token (TOK_GE); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }


{NOTIDENT}     { lexer_badtoken (yytext); return yylval_token (TOK_IDENT); }
{NOTCHAR}       { lexer_badtoken(yytext); return yylval_token (TOK_CHARCON); }
{NOTSTRING}     { lexer_badtoken(yytext); return yylval_token(TOK_STRINGCON); }
.               { lexer_badchar (*yytext); }

%%
