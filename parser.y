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
%token TOK_ROOT TOK_ORD TOK_CHR TOK_DECLID TOK_VARDECL
%token TOK_FUNCTION TOK_PROTOTYPE TOK_PARAM TOK_RETURNVOID

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

program : program structdef   { $$ = adopt1($1,$2); }
        | program function    { $$ = adopt1 ($1,$2); }
        | program statement   { $$ = adopt1($1,$2); }
        | program error '}'   { $$ = $1; }
        | program error ';'   { $$ = $1; }
        |                     { $$ = new_parseroot(); }
        ;

structdef   : TOK_STRUCT TOK_IDENT '{' fields '}'
              { $$ = adopt2($1,$2,$4); $2->symbol=TOK_TYPEID;free_ast2($3,$5);}
            | TOK_STRUCT TOK_IDENT '{' '}'  {$$ = adopt1($1,$2); free_ast2($3,$4);}

fields      : fields fielddec ';'{$2->symbol=TOK_TYPEID; $$=adopt1($1,$2);}
            | fielddec ';'       {$$=$1;free_ast($2);}
            ;

fielddec    : basetype TOK_ARRAY TOK_IDENT  {$3->symbol=TOK_FIELD; $$=adopt2($1,$2,$3);}
            | basetype TOK_IDENT {$2->symbol=TOK_FIELD; $$=adopt1($1,$2);}
            ;

basetype    : TOK_VOID        { $$ = $1; }
            | TOK_BOOL        { $$ = $1; }
            | TOK_CHAR        { $$ = $1; }
            | TOK_INT         { $$ = $1; }
            | TOK_STRING      { $$ = $1; }
            | TOK_IDENT       { $1->symbol=TOK_TYPEID; $$ = $1; }
            ;

function    : identdecl params ')' block      {$$=funcCheck($1,$2,$4); free_ast($3);}
            | identdecl params ')' ';'        {$$=funcCheck($1,$2,$4); free_ast($3);}
            ;

params      : '('                                          {$1->symbol=TOK_PARAM; $$=$1;}
            | params ',' identdecl                         {adopt1($1,$3); free_ast($2);}
            | params identdecl                             {adopt1($1,$2);}
            ;

identdecl   : basetype TOK_ARRAY TOK_IDENT       {$3->symbol=TOK_DECLID; $$=adopt2($1,$2,$3);}
            | basetype TOK_IDENT                 {$2->symbol=TOK_DECLID; $$=adopt1($1,$2);}
            ;

block       : '{' block statement ';' '}'
            | statement ';'
            ;

statement   : block       {$$=$1;}
            | vardecl     {$$=$1;}
            | while       {$$=$1;}
            | ifelse      {$$=$1;}
            | return      {$$=$1;}
            | expr ';'    {$$=$1;free_ast($2);}
            ;

vardecl     : identdecl '=' expr ';'  {$2->symbol=TOK_VARDECL; $$=adopt2($2,$1,$3); free_ast($4); }
            ;

while       : TOK_WHILE '(' expr ')' statement  {$$=adopt2($1,$3,$5);free_ast2($2,$4);}
            ;

ifelse      : TOK_IF '(' expr ')' statement %prec TOK_IF  {$$=adopt2($1,$3,$5);free_ast2($2,$4);}
            | TOK_IF '(' expr ')' statement TOK_ELSE statement  {$1->symbol=TOK_IFELSE; $$=adopt3($1,$3,$5,$7); free_ast2($2,$4);}
            ;

return      : TOK_RETURN expr ';' {$$=adopt1($1,$2);free_ast($3);}
            | TOK_RETURN ';'      {$1->symbol=TOK_RETURNVOID; $$=$1;free_ast($2);}
            ;

expr        : binop
            | unop
            | allocator
            | call
            | '(' expr ')'
            | variable
            | constant
            ;

binop       : expr '=' expr     {$$=adopt2($2,$1,$3);}
            | expr '+' expr     {$$=adopt2($2,$1,$3);}
            | expr '-' expr     {$$=adopt2($2,$1,$3);}
            | expr '*' expr     {$$=adopt2($2,$1,$3);}
            | expr '/' expr     {$$=adopt2($2,$1,$3);}
            | expr '%' expr     {$$=adopt2($2,$1,$3);}
            | expr TOK_EQ expr  {$$=adopt2($2,$1,$3);}
            | expr TOK_NE expr  {$$=adopt2($2,$1,$3);}
            | expr TOK_GE expr  {$$=adopt2($2,$1,$3);}
            | expr TOK_GT expr  {$$=adopt2($2,$1,$3);}
            | expr TOK_LE expr  {$$=adopt2($2,$1,$3);}
            | expr TOK_LT expr  {$$=adopt2($2,$1,$3);}
            ;

unop        : '+' expr %prec TOK_POS     {$1->symbol=TOK_POS; $$=adopt1($1,$2);}
            | '-' expr %prec TOK_NEG     {$1->symbol=TOK_NEG; $$=adopt1($1,$2);}
            | '!' expr      {$$=adopt1($1,$2);}
            | TOK_ORD expr  {$$=adopt1($1,$2);}
            | TOK_CHR expr  {$$=adopt1($1,$2);}
            ;

allocator   : TOK_NEW TOK_IDENT '(' ')'
            | TOK_NEW TOK_STRING '(' expr ')'
            | TOK_NEW basetype '[' expr ']'
            ;

call        : TOK_IDENT callparams ')'
            ;

callparams  : '('
            | callparams ',' expr
            | callparams expr
            ;

variable    : TOK_IDENT
            | expr '[' expr ']'
            | expr '.' TOK_IDENT  {$3->symbol=TOK_FIELD;}
            ;

constant    : TOK_INTCON        {$$=$1}
            | TOK_CHARCON       {$$=$1}
            | TOK_STRINGCON     {$$=$1}
            | TOK_FALSE         {$$=$1}
            | TOK_TRUE          {$$=$1}
            | TOK_NULL          {$$=$1}
            ;


%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

void swapSym(astree* node, int tcode){
    node->symbol=tcode;
}

//static void* yycalloc (size_t size) {
//   void* result = calloc (1, size);
//   assert (result != NULL);
//   return result;
//}
