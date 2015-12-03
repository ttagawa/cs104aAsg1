//John King joscking
//Tyler Tagawa ttagawa 
#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
using namespace std;

#include "auxlib.h"

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
     ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
     ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval,
     ATTR_const, ATTR_vreg, ATTR_vaddr, ATTR_bitset_size
   };

using attr_bitset1 = bitset<ATTR_bitset_size>;

struct astree {
   attr_bitset1 attribs;
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   astree (int symbol, int filenr, int linenr,
           int offset, const char* clexinfo);

};

string getAttrs(astree* root);
// Append one child to the vector of children.
astree* adopt1 (astree* root, astree* child);

// Append two children to the vector of children.
astree* adopt2 (astree* root, astree* left, astree* right);

// Append three children to the vector of children.
astree* adopt3 (astree* root, astree* c1, astree* c2, astree* c3);

astree* appendAdopt(astree* parent, astree* child);

//Create new root node and append three children.
astree* funcCheck (astree* root, astree* param, astree* child);

// Dump an astree to a FILE.
void dump_astree (FILE* outfile, astree* root);

// Debug print an astree.
void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep);

// Recursively free an astree.
void free_ast (astree* tree);

// Recursively free two astrees.
void free_ast2 (astree* tree1, astree* tree2);

//RCSH("$Id: astree.h,v 1.4 2015-04-09 19:31:47-07 - - $")
#endif
