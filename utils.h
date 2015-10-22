// $Id: utils.h,v 1.9 2015-07-09 18:10:52-07 - - $

#ifndef __UTILS_H__
#define __UTILS_H__

// Lex and Yacc interface utility.

#include <string>
#include <vector>
using namespace std;

#include <stdio.h>

#include "astree.h"
#include "auxlib.h"

//extern int yy_flex_debug;
//extern int yydebug;

int yyparse();
void yyerror (const char* message);

struct lexer {
   static bool interactive;
   static location lloc;
   static size_t last_yyleng;
   static vector<string> filenames;
   static const string* filename (int filenr);
   static void newfilename (const string& filename);
   static void advance();
   static void newline();
   static void badchar (unsigned char bad);
   static void badtoken (char* lexeme);
   static void include();
};

struct parser {
   static astree* root;
   static const char* get_tname (int symbol);
};

#define YYSTYPE astree*
#include "yyparse.h"

#endif
