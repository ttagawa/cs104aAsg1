#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <unordered_map>
#include <bitset>
#include <vector>
#include <string>
#include "lyutils.h"

using namespace std;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
      ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
      ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval,
      ATTR_const, ATTR_vreg, ATTR_vaddr, ATTR_bitset_size
    };

struct symbol;
using attr_bitset = bitset<ATTR_bitset_size>;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = symbol_table::value_type;

struct symbol {
  attr_bitset attributes;
  symbol_table* fields;
  size_t filenr;
  size_t linenr;
  size_t offset;
  size_t block_nr;
  vector<symbol*>* parameters;
};

void travCompare(astree* root);

void traverseFunc(astree* root, int symbol);

bool typechkArr(astree* n, astree* n1);

void checkCall(astree* curr);

void checkDecl(astree* root);

void dumpToFile(FILE* outfile, symbol* sym, astree* root);

void dumpParams(symbol* sym, astree* root);

void insertArr(astree* node, astree* node1);

symbol *create_symbol (astree *sym_node);

bool lookup(const string* x, astree* node);

symbol* lookupSym(const string* x);

int getIdentReturn(symbol* sym);

int getReturnType(astree* root);

string getAttr(symbol* sym);

void travVardecl(astree* root);

void traverseAstree(astree* root);

#endif
