#include "symtab.h"

size_t blockcount = 0;
vector<string> attrArray { "void", "bool", "char", "int", "null",
      "string", "struct", "array", "function",
      "variable", "field", "typeid", "param", "lval",
      "const", "vreg", "vaddr", "bitset_size"
    };

vector<symbol_table*> symbol_stack;

symbol *create_symbol (astree *sym_node){
  if(sym_node==NULL) return NULL;
  symbol *newSym = new symbol();
  newSym->fields = NULL;
  newSym->filenr = sym_node->filenr;
  newSym->linenr = sym_node->linenr;
  newSym->offset = sym_node->offset;
  newSym->block_nr = blockcount;
  newSym->parameters = NULL;
  return newSym;
}

string getAttr(symbol* sym){
  string attrstr;
  for(size_t i = 0; i<sym->attributes.size(); i++){
    if(sym->attributes[i]==1) attrstr+=attrArray[i]+" ";
  }
  return attrstr;
}

void dumpToFile(FILE* outfile, symbol* sym, astree* root){
  string spaces;
  string attrs = getAttr(sym);
  for(size_t i = 0; i < sym->block_nr; i++){
    spaces += "   ";
  }
  fprintf(outfile, "%s %s (%zu.%zu.%zu) {%zu} %s\n", spaces.c_str(),
          root->lexinfo->c_str(),sym->filenr, sym->linenr, sym->offset,
                    sym->block_nr, attrs.c_str());
}

void travVardecl(astree* root){
  astree* node = root->children[0];
  astree* node1 = root->children[1];
  int sym = node->symbol;
  switch(sym){
    case TOK_INT:
      if(node1->symbol==TOK_INTCON){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_int);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.empty()){
          symbol_table tab;
          tab.insert(symbol_entry(node->children[0]->lexinfo,newSym));
        }else{
          symbol_stack.back()->insert(symbol_entry(node->children[0]->
                                                  lexinfo,newSym));
        }
        dumpToFile(file_sym, newSym, node->children[0]);
      }
      break;

    case TOK_CHAR:
      if(node1->symbol==TOK_CHARCON){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_char);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.empty()){
          symbol_table tab;
          tab.insert(symbol_entry(node->children[0]->lexinfo,newSym));
        }else{
          symbol_stack.back()->insert(symbol_entry(node->children[0]->
                                                  lexinfo,newSym));
        }
        dumpToFile(file_sym, newSym, node->children[0]);
      }
      break;
  }
}

void traverseAstree(astree* root){
  for (size_t a = 0; a < root->children.size(); ++a){
    int sym = root->children[a]->symbol;
    switch(sym){
      case TOK_VARDECL: travVardecl(root->children[a]); break;
    }
  }
}
