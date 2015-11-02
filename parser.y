%{
// Dummy parser for scanner project.
#include <assert.h>
#include "lyutils.h"
#include "astree.h"
%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ROOT TOK_ORD TOK_CHR

%right  TOK_IF TOK_ELSE
%right  '='
%left   TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left   '+' '-'
%left   '*' '/' '%'
%right  TOK_POS TOK_NEG '!' TOK_NEW TOK_ORD TOK_CHR
%left   '.' '[' '('
%nonassoc "parens"

%start start

%%

start   : program             {yyparse_astree = $1}

program : program structdef   { $$ = adopt1 ($1, $2); }
        | program function    { $$ = adopt1 ($1, $2); }
        | program statement   { $$ = adopt1 ($1, $2); }
        | program error ’}’   { $$ = $1; }
        | program error ’;’   { $$ = $1; }
        |                     { $$ = new_parseroot (); }
        ;

structdef   : TOK_STRUCT TOK_IDENT '{' fields '}'
{ $$ = adopt2($1,$2,$4); $2->symbol=TOK_TYPEID;free_ast2($3,$5);}
            | TOK_STRUCT TOK_IDENT '{' '}'  {$$ = adopt1($1,$2); free_ast2($3,$4);}

fields      : fields fielddec ';'{$2->symbol=TOK_TYPEID; $$=adopt1($1,$2);}
            | fielddec ';'       {$$=$1;free_ast($2);}
            ;

fielddec    : basetype TOK_ARRAY TOK_IDENT ';'  {$3->symbol=TOK_FIELD; $$=adopt2($1,$2,$3); free_ast($4);}
            | basetype TOK_IDENT ';' {$2->symbol=TOK_FIELD; $$=adopt1($1,$2); free_ast($3);}
            ;


basetype    : TOK_VOID        { $$ = $1; }
            | TOK_BOOL        { $$ = $1; }
            | TOK_CHAR        { $$ = $1; }
            | TOK_INT         { $$ = $1; }
            | TOK_STRING      { $$ = $1; }
            | TOK_IDENT       { $$ = $1; $1->symbol=TOK_TYPEID; }
                    ;

token   : '(' | ')' | '[' | ']' | '{' | '}' | ';' | ',' | '.'
        | '=' | '+' | '-' | '*' | '/' | '%' | '!'
        | TOK_VOID | TOK_BOOL | TOK_CHAR | TOK_INT | TOK_STRING
        | TOK_IF | TOK_ELSE | TOK_WHILE | TOK_RETURN | TOK_STRUCT
        | TOK_FALSE | TOK_TRUE | TOK_NULL | TOK_NEW | TOK_ARRAY
        | TOK_EQ | TOK_NE | TOK_LT | TOK_LE | TOK_GT | TOK_GE
        | TOK_IDENT | TOK_INTCON | TOK_CHARCON | TOK_STRINGCON
        | TOK_ROOT | TOK_ORD | TOK_CHR
        ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

//static void* yycalloc (size_t size) {
//   void* result = calloc (1, size);
//   assert (result != NULL);
//   return result;
//}
