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

bool lookup(const string* x){
  for(auto i=symbol_stack.crbegin();i!=symbol_stack.crend();++i){
    symbol_table* pos = *i;
    auto result = pos->find(x);
    if(result!=pos->cend()) return true;
  }
  return false;
}

symbol* lookupSym(const string* x){
  for(auto i=symbol_stack.crbegin();i!=symbol_stack.crend();++i){
    symbol_table* pos = *i;
    auto result = pos->find(x);
    if(result!=pos->cend()) return result->second;
  }
  return NULL;
}

int getIdentReturn(symbol* sym){
  size_t i;
  for(i = 0;i<sym->attributes.size();i++){
    int tok = sym->attributes[i];
    if (tok == 1) break;
  }
  string tok = attrArray[i];
  if(tok == "int") return TOK_INTCON;
  else if (tok == "char") return TOK_CHARCON;
  else if (tok == "string") return TOK_STRINGCON;
  else if (tok == "bool") return TOK_BOOL;
  else if (tok == "struct") return TOK_STRUCT;
  return 0;
}

int getReturnType(astree* root){
  int sym = root->symbol;
  switch(sym){
    case TOK_INTCON:return sym;break;
    case TOK_CHARCON:return sym;break;
    case TOK_BOOL:return sym;break;
    case TOK_IDENT:
        if(lookup(root->lexinfo)){
          symbol* val = lookupSym(root->lexinfo);
          return getIdentReturn(val);
        }
        break;
    case '+': case '-': case '*': case '/': case '%':
      {
        int left = getReturnType(root->children[0]);
        int right = getReturnType(root->children[1]);
        if(right == left){
          return right;
        }else{
          errprintf("%zu.%zu.%zu Type check error: %s does not match %s\n"
          ,root->children[0]->filenr,root->children[0]->linenr,
          root->children[0]->offset,get_yytname(left),get_yytname(right));
          return right;
        }
        break;
      }
    case TOK_NEWARRAY:
      return TOK_NEWARRAY;
      break;

  }
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

void insertArr(astree* node, astree* node1){
    if(typechkArr(node, node1)){
      int arrSym = node->symbol;
      switch (arrSym){
        case TOK_INT:
        {
          symbol *newSym = create_symbol(node->children[1]);
          newSym->attributes.set(ATTR_int);
          newSym->attributes.set(ATTR_array);
          newSym->attributes.set(ATTR_lval);
          newSym->attributes.set(ATTR_variable);
          if(symbol_stack.empty()){
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[1]->lexinfo,newSym));
            symbol_stack.push_back(tab);
          }else{
            symbol_stack.back()->insert(symbol_entry(node->children[1]->
                                                  lexinfo,newSym));
        }
        dumpToFile(file_sym, newSym, node->children[1]);
        break;
        }
        case TOK_CHAR:
        {
          symbol *newSym = create_symbol(node->children[1]);
          newSym->attributes.set(ATTR_char);
          newSym->attributes.set(ATTR_array);
          newSym->attributes.set(ATTR_lval);
          newSym->attributes.set(ATTR_variable);
          if(symbol_stack.empty()){
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[1]->lexinfo,newSym));
            symbol_stack.push_back(tab);
          }else{
            symbol_stack.back()->insert(symbol_entry(node->children[1]->
                                                    lexinfo,newSym));
          }
          dumpToFile(file_sym, newSym, node->children[1]);
          break;
        }

        case TOK_BOOL:
        {
          symbol *newSym = create_symbol(node->children[1]);
          newSym->attributes.set(ATTR_bool);
          newSym->attributes.set(ATTR_lval);
          newSym->attributes.set(ATTR_array);
          newSym->attributes.set(ATTR_variable);
          if(symbol_stack.empty()){
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[1]->lexinfo,newSym));
            symbol_stack.push_back(tab);
          }else{
            symbol_stack.back()->insert(symbol_entry(node->children[1]->
                                                    lexinfo,newSym));
          }
          dumpToFile(file_sym, newSym, node->children[1]);
          break;
        }
      }
    }
}

bool typechkArr ( astree* node, astree* node1){
  int right2 = getReturnType(node1->children[1]);
  int right1 = getReturnType(node1->children[0]);
  if(right2!=TOK_INTCON){
    errprintf("%zu.%zu.%zu Array size allocator not of type int.\n",
    node1->children[1]->filenr, node1->children[1]->linenr,
    node1->children[1]->offset);
    return false;
  }else if(node->symbol == right1){
    return true;
  }else{
    errprintf("%zu.%zu.%zu Type check error: %s does not match %s\n"
    ,node->children[0]->filenr,node->children[0]->linenr,
    node->children[0]->offset,get_yytname(node->symbol),
                                   get_yytname(right1));
    return false;
  }
}

void travVardecl(astree* root){
  astree* node = root->children[0];
  astree* node1 = root->children[1];
  int sym = node->symbol;
  int otherSym = getReturnType(node1);
  switch(sym){
    case TOK_INT:
      if(otherSym==TOK_INTCON){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_int);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.push_back(tab);
        }else{
          symbol_stack.back()->insert(symbol_entry(node->children[0]->
                                                  lexinfo,newSym));
        }
        dumpToFile(file_sym, newSym, node->children[0]);
      }else if(otherSym == TOK_NEWARRAY){
        insertArr(node, node1);
      }
      break;

    case TOK_CHAR:
      if(otherSym==TOK_CHARCON){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_char);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.push_back(tab);
        }else{
          symbol_stack.back()->insert(symbol_entry(node->children[0]->
                                                  lexinfo,newSym));
        }
        dumpToFile(file_sym, newSym, node->children[0]);
      } else if(otherSym == TOK_NEWARRAY){
        insertArr(node, node1);
      }
      break;

    case TOK_BOOL:
      if(otherSym==TOK_TRUE || otherSym==TOK_FALSE){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_bool);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.push_back(tab);
        }else{
          symbol_stack.back()->insert(symbol_entry(node->children[0]->
                                                  lexinfo,newSym));
        }
        dumpToFile(file_sym, newSym, node->children[0]);
      } else if (otherSym == TOK_NEWARRAY){
        insertArr(node, node1);
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
