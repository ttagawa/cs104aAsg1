//John King joscking
//Tyler Tagawa ttagawa
#include <string.h>
#include "lyutils.h"
#include "symtab.h"
size_t blockcount = 0;
vector<string> attrArray { "void", "bool", "char", "int", "null",
      "string", "struct", "array", "function",
      "variable", "field", "typeid", "param", "lval",
      "const", "vreg", "vaddr", "bitset_size"
    };

vector<symbol_table*> symbol_stack;
vector<const string*> strvec;
int flag = 0;

symbol *create_symbol (astree *sym_node){
  if(sym_node==NULL) return NULL;
  symbol *newSym = new symbol();
  newSym->fields = NULL;
  newSym->type_id = "";
  newSym->filenr = sym_node->filenr;
  newSym->linenr = sym_node->linenr;
  newSym->offset = sym_node->offset;
  newSym->block_nr = blockcount;
  newSym->parameters = NULL;
  return newSym;
}

bool lookup(const string* x){
  for(auto i=symbol_stack.crbegin();i!=symbol_stack.crend();++i){
    if(*i == nullptr){
      continue;
    }
    symbol_table* pos = *i;
    auto result = pos->find(x);
    if(result!=pos->cend()) return true;

  }
  return false;
}

symbol* lookupSym(const string* x){
  for(auto i=symbol_stack.crbegin();i!=symbol_stack.crend();++i){
    if(*i == nullptr){
      continue;
    }
    symbol_table* pos = *i;
    auto result = pos->find(x);
    if(result!=pos->cend()) return result->second;
  }
  return NULL;
}

symbol* lookupFields(const string* x,symbol_table* tab){
  auto result = tab->find(x);
  if(result!=tab->cend()) return result->second;
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
    case TOK_NULL: return sym; break;
    case TOK_INTCON:return sym;break;
    case TOK_CHARCON:return sym;break;
    case TOK_BOOL:return sym;break;
    case TOK_IDENT:
        if(lookup(root->lexinfo)){
          symbol* val = lookupSym(root->lexinfo);
          return getIdentReturn(val);
        }else{
          errprintf("%zu.%zu.%zu"
          " Variable being referenced is undefined\n",root->filenr,
          root->linenr,root->offset);
          return -1;
        }
        break;
    case '+': case '-': case '*': case '/': case '%':
      {
        int left = getReturnType(root->children[0]);
        int right = getReturnType(root->children[1]);
        if(right == left && right == TOK_INTCON){
          return right;
        }else{
          errprintf("%zu.%zu.%zu Type check error: %s "
          "does not match %s\n"
          ,root->children[0]->filenr,root->children[0]->linenr,
          root->children[0]->offset,get_yytname(left)
          ,get_yytname(right));
          return right;
        }
        break;
      }
    case TOK_NEWARRAY:
      return TOK_NEWARRAY;
      break;
    case TOK_NEWSTRING:
      return TOK_NEWSTRING;
      break;
    case TOK_CALL:
      checkCall(root);
      return getReturnType(root->children[0]);
      break;
    case TOK_POS: case TOK_NEG:
      return getReturnType(root->children[0]);
      break;
    case TOK_NEW:
      return getReturnType(root->children[0]);
      break;
    case TOK_TYPEID:
      return TOK_TYPEID;
      break;
    case '.':
      if(lookup(root->children[0]->lexinfo)){
        symbol* tempSym = lookupSym(root->children[0]->lexinfo);
        string structType = tempSym->type_id;
        if(structType == ""){
          errprintf("%zu.%zu.%zu Referenced struct not defined\n",
          root->children[0]->filenr,root->children[0]->linenr,
          root->children[0]->offset);
          return -1;
        }
        if(lookup(&structType)){
          symbol* structSym = lookupSym(&structType);
          symbol_table* tab = structSym->fields;
          auto fi = tab->find(root->children[1]->lexinfo);
          if(fi!=tab->cend()){
            symbol* structField = fi->second;
            return getIdentReturn(structField);
          }else{
            errprintf("%zu.%zu.%zu Field not defined\n",
            root->children[1]->filenr,root->children[1]->linenr,
            root->children[1]->offset);
          }
        }
      }else{
        errprintf("%zu.%zu.%zu Referenced struct not defined\n",
        root->children[0]->filenr,root->children[0]->linenr,
        root->children[0]->offset);
      }
      return -1;
      break;
    case TOK_ORD:
      if(getReturnType(root->children[0])==TOK_CHARCON){
        return TOK_INTCON;
      }else{
        errprintf("%zu.%zu.%zu Ord must be called on type char\n",
        root->children[0]->filenr,root->children[0]->linenr,
        root->children[0]->offset);
      }
      break;
    case TOK_CHR:
      if(getReturnType(root->children[0])==TOK_INTCON){
        return TOK_CHARCON;
      }else{
        errprintf("%zu.%zu.%zu Chr must be called on type int\n",
        root->children[0]->filenr,root->children[0]->linenr,
        root->children[0]->offset);
      }
      break;
  }
}

string getAttr(symbol* sym){
  string attrstr = "";
  for(size_t i = 0; i<sym->attributes.size(); i++){
    if(sym->attributes[i]==1){
      if(i == ATTR_typeid) continue;
      if(sym->attributes[ATTR_struct]==1 && i==ATTR_struct){
        attrstr+=attrArray[i]+" ";
        attrstr+="\""+sym->type_id+"\""+" ";
        continue;
      }
      attrstr+=attrArray[i]+" ";
      if(sym->attributes[ATTR_field]==1 && i==ATTR_field){
        attrstr+="{"+sym->type_id+"}"+" ";
      }
    }
  }
  return attrstr;
}

void dumpToFile(FILE* outfile, symbol* sym, astree* root){
  string spaces;
  string attrs = getAttr(sym);
  for(size_t i = 0; i < sym->block_nr; i++){
    spaces += "   ";
  }
  if(sym->attributes[ATTR_typeid]==1){
    fprintf(outfile, "%s %s (%zu.%zu.%zu) %s\n", spaces.c_str(),
            root->lexinfo->c_str(),sym->filenr,
            sym->linenr, sym->offset,
            attrs.c_str());
    return;
  }
  fprintf(outfile, "%s %s (%zu.%zu.%zu) {%zu} %s\n", spaces.c_str(),
          root->lexinfo->c_str(),sym->filenr, sym->linenr, sym->offset,
                    sym->block_nr, attrs.c_str());
}

void dumpParams(symbol* sym, astree* root){
  for(size_t i = 0;i<sym->parameters->size();i++){
    symbol* param = sym->parameters->at(i);
    if(symbol_stack.back() == nullptr || symbol_stack.empty()){
      symbol_table* tab = new symbol_table();
      tab->insert(symbol_entry(root->children[i]->children[0]->
                                            lexinfo,param));
      symbol_stack.pop_back();
      symbol_stack.push_back(tab);
    }else{
      symbol_stack.back()->insert(symbol_entry(root->children[i]->
                                  children[0]->lexinfo,param));
    }
    dumpToFile(file_sym, param, root->children[i]->children[0]);
  }
  fprintf(file_sym, "\n");
}

void dumpFields(symbol* sym, astree* root){
  for(size_t i = 1;i<root->children.size();i++){
    astree* temp = root->children[i]->children[0];
    symbol* fieldsym = lookupFields(temp->lexinfo,sym->fields);
    fieldsym->type_id = sym->type_id;
    string attribs = getAttr(fieldsym);
    fprintf(file_sym, "   %s (%zu.%zu.%zu) %s\n",temp->lexinfo->c_str(),
    temp->filenr,temp->linenr,temp->offset,attribs.c_str());
  }
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
          if(symbol_stack.back() == nullptr || symbol_stack.empty()){
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[1]
                                    ->lexinfo,newSym));
            symbol_stack.pop_back();
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
          if(symbol_stack.back() == nullptr || symbol_stack.empty()){
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[1]
                                    ->lexinfo,newSym));
            symbol_stack.pop_back();
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
          if(symbol_stack.back() == nullptr || symbol_stack.empty()) {
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[1]
                                    ->lexinfo,newSym));
            symbol_stack.pop_back();
            symbol_stack.push_back(tab);
          }else{
            symbol_stack.back()->insert(symbol_entry(node->children[1]->
                                                    lexinfo,newSym));
          }
          dumpToFile(file_sym, newSym, node->children[1]);
          break;
        }
        case TOK_STRING:
        {
          symbol *newSym = create_symbol(node->children[1]);
          newSym->attributes.set(ATTR_string);
          newSym->attributes.set(ATTR_lval);
          newSym->attributes.set(ATTR_array);
          newSym->attributes.set(ATTR_variable);
          if(symbol_stack.back() == nullptr || symbol_stack.empty()){
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[1]->
                                        lexinfo,newSym));
            symbol_stack.pop_back();
            symbol_stack.push_back(tab);
          }else{
           symbol_stack.back()->insert(symbol_entry(node->children[1]->
                                                    lexinfo,newSym));
          }
          dumpToFile(file_sym, newSym, node->children[1]);
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

void checkCall(astree* curr){
  if(lookup(curr->children[0]->lexinfo)){
    symbol *fnSym = lookupSym(curr->children[0]->lexinfo);
    if(curr->children.size()-1!=fnSym->parameters->size()){
      errprintf("%zu.%zu.%zu Parameters of defined function"
      " do not match\n",
      curr->children[0]->filenr,curr->children[0]->linenr,
      curr->children[0]->offset);
    }else{
      for(size_t i = 0;i<fnSym->parameters->size();i++){
        int paramType = getIdentReturn(fnSym->parameters->at(i));
        int callParam = getReturnType(curr->children[i+1]);
        if(paramType!=callParam){
          if(callParam == TOK_NULL) continue;
          errprintf("%zu.%zu.%zu Argument type does not match"
          " defined function argument type.\n",curr->
          children[i+1]->filenr,curr->children[i+1]->
          linenr,curr->children[i+1]->offset);
        }
      }
    }
  }else{
    errprintf("%zu.%zu.%zu Function not defined\n",
    curr->children[0]->filenr,curr->children[0]->linenr,
    curr->children[0]->offset);
  }
}

void checkDecl(astree* root){
  astree* node = root->children[0];
  if(lookup(node->lexinfo)){
    errprintf("%zu.%zu.%zu Error: Redeclaration of variable %s\n"
    ,node->filenr,node->linenr,node->offset, node->lexinfo->c_str());
  }
}

void travVardecl(astree* root){
  astree* node = root->children[0];
  astree* node1 = root->children[1];
  int sym = node->symbol;
  int otherSym = getReturnType(node1);
  switch(sym){
    case TOK_INT:
      checkDecl(node);
      if(otherSym==TOK_INTCON || otherSym==TOK_NULL){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_int);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.pop_back();
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
      checkDecl(node);
      if(otherSym==TOK_CHARCON || otherSym==TOK_NULL){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_char);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.pop_back();
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
      checkDecl(node);
      if(otherSym==TOK_TRUE ||
        otherSym==TOK_FALSE || otherSym==TOK_NULL){
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_bool);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.pop_back();
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
    case TOK_STRING:
      checkDecl(node);
      if(otherSym == TOK_NEWSTRING){
        if(node1->children[0]->symbol == TOK_INTCON ||
          otherSym==TOK_NULL){
          symbol *newSym = create_symbol(node->children[0]);
          newSym->attributes.set(ATTR_string);
          newSym->attributes.set(ATTR_lval);
          newSym->attributes.set(ATTR_variable);
          if(symbol_stack.back() == nullptr || symbol_stack.empty()){
            symbol_table* tab = new symbol_table();
            tab->insert(symbol_entry(node->children[0]
              ->lexinfo,newSym));
            symbol_stack.pop_back();
            symbol_stack.push_back(tab);
          }else{
           symbol_stack.back()->insert(symbol_entry(node->children[0]->
                                                    lexinfo,newSym));
          }
          dumpToFile(file_sym, newSym, node->children[0]);
        }else{
          errprintf("%zu.%zu.%zu String size allocator not of "
          "type int.\n",
          node1->children[1]->filenr, node1->children[1]->linenr,
          node1->children[1]->offset);
        }
      }else if (otherSym == TOK_STRINGCON || otherSym==TOK_NULL){
        if(otherSym!=TOK_NULL)
          strvec.push_back(node1->lexinfo);
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_string);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.pop_back();
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
    case TOK_TYPEID:
      if(otherSym == TOK_TYPEID || otherSym==TOK_NULL
        || otherSym == TOK_STRUCT){
        if(node1->symbol != TOK_CALL){
          string leftStr = *node->lexinfo;
          string rightStr = *node1->children[0]->lexinfo;
          if(leftStr != rightStr){
            errprintf("%zu.%zu.%zu Type mismatch between constructor "
                      "and declaration.\n",
            node1->children[1]->filenr, node1->children[1]->linenr,
            node1->children[1]->offset);
          }
        }else{
          if(lookup(node1->children[0]->lexinfo)){
            symbol* funcSym = lookupSym(node1->children[0]->lexinfo);
            string leftStr = *node->lexinfo;
            string rightStr = funcSym->type_id;
            if(leftStr != rightStr){
              errprintf("%zu.%zu.%zu Type mismatch between constructor "
                        "and declaration.\n",
              node1->children[1]->filenr, node1->children[1]->linenr,
              node1->children[1]->offset);
            }
          }
        }
        symbol *newSym = create_symbol(node->children[0]);
        newSym->attributes.set(ATTR_struct);
        newSym->attributes.set(ATTR_lval);
        newSym->attributes.set(ATTR_variable);
        newSym->type_id = *node->lexinfo;
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(node->children[0]->lexinfo,newSym));
          symbol_stack.pop_back();
          symbol_stack.push_back(tab);
        }else{
          symbol_stack.back()->insert(symbol_entry(node->children[0]->
                                                  lexinfo,newSym));
        }

        dumpToFile(file_sym, newSym, node->children[0]);
      }else{
        errprintf("%zu.%zu.%zu Variable not allocated correctly.\n",
        node1->children[1]->filenr, node1->children[1]->linenr,
        node1->children[1]->offset);
      }
      break;
    case TOK_IDENT:
      if(lookup(node->lexinfo)){
        symbol* newSym = lookupSym(node->lexinfo);
        int type = getIdentReturn(newSym);
        if(type==TOK_STRUCT&&otherSym==TOK_TYPEID){
          if(newSym->type_id!=*node1->children[0]->lexinfo){
            errprintf("%zu.%zu.%zu Variable being reassigned wrong "
            "type",node->filenr,node->linenr,node->offset);
          }
        }
      }else{
        errprintf("%zu.%zu.%zu"
        " Variable being referenced is undefined\n",node->filenr,
        node->linenr,node->offset);
      }
    }
}

void travCompare(astree* root){
  int sym = root->symbol;
  int left = getReturnType(root->children[0]);
  int right = getReturnType(root->children[1]);
  switch (sym){
    case TOK_EQ: case TOK_NE:
      if( left == right || left == TOK_NULL || right == TOK_NULL){
        return;
      } else {
        errprintf("%zu.%zu.%zu Comparison is not of same type.\n",
        root->filenr, root->linenr,root->offset);
      }
      break;
    case TOK_GE: case TOK_GT: case TOK_LE: case TOK_LT:
      if ( left == right && (right == TOK_INTCON ||
                              right == TOK_CHARCON)){
        return;
      }else {
       errprintf("%zu.%zu.%zu Comparison is not of same type"
                " or is not of primitive type.\n",
                root->filenr, root->linenr,root->offset);
      }
      break;
  }
}

void validCompare(astree* node){
  int sym = node->children[0]->symbol;
  switch (sym){
    case TOK_GE: case TOK_GT: case TOK_LE: case TOK_LT: case TOK_EQ:
    case TOK_NE:
       travCompare(node->children[0]);
       break;
    default:
       errprintf("%zu.%zu.%zu Comparison is not valid.\n",
       node->filenr, node->linenr,node->offset);
  }
}

void traverseAstree(astree* root){
  if( flag == 0 ){
    symbol_stack.push_back(nullptr);
    flag++;
  }
  for (size_t a = 0; a < root->children.size(); ++a){
    int sym = root->children[a]->symbol;
    astree* curr = root->children[a];
    switch(sym){
      case TOK_VARDECL:
        travVardecl(curr);
        break;
      case TOK_IF: case TOK_WHILE:
         validCompare(curr);
         symbol_stack.push_back(nullptr);
         blockcount++;
         traverseAstree(curr->children[1]);
         blockcount--;
         symbol_stack.pop_back();
         break;
      case TOK_IFELSE:
        validCompare(curr);
        for(size_t i = 0; i<curr->children.size(); i++){
          symbol_stack.push_back(nullptr);
          blockcount++;
          traverseAstree(curr->children[i]);
          blockcount--;
          symbol_stack.pop_back();
        }
        break;
      case TOK_FUNCTION:
      {
        symbol* newFunc = create_symbol(root->children[a]);
        blockcount++;
        newFunc->attributes.set(ATTR_function);
        switch(curr->children[0]->symbol){
          case TOK_INT:
            newFunc->attributes.set(ATTR_int);
            break;
          case TOK_STRING:
            newFunc->attributes.set(ATTR_string);
            break;
          case TOK_CHAR:
            newFunc->attributes.set(ATTR_char);
            break;
          case TOK_BOOL:
            newFunc->attributes.set(ATTR_bool);
            break;
          case TOK_TYPEID:
            newFunc->attributes.set(ATTR_struct);
            newFunc->attributes.set(ATTR_typeid);
            newFunc->type_id =*root->children[a]->children[0]->lexinfo;
            break;
        }
        newFunc->parameters = new vector<symbol*>;
        for(size_t i = 0; i<curr->children[1]->children.size(); i++){
          astree* param_node = curr->children[1]->children[i];
          symbol* param = create_symbol(param_node->children[0]);
          param->attributes.set(ATTR_variable);
          param->attributes.set(ATTR_lval);
          param->attributes.set(ATTR_param);
          int paramSym = param_node->symbol;
          switch(paramSym){
            case TOK_INT:
              param->attributes.set(ATTR_int);
              break;
            case TOK_CHAR:
              param->attributes.set(ATTR_char);
              break;
            case TOK_STRING:
              param->attributes.set(ATTR_string);
              break;
            case TOK_BOOL:
              param->attributes.set(ATTR_bool);
              break;
          }
          newFunc->parameters->push_back(param);
        }
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(curr->children[0]->children[0]->
                                                 lexinfo,newFunc));
          symbol_stack.pop_back();
          symbol_stack.push_back(tab);
        }else{
          symbol_stack.back()->insert(symbol_entry(curr->children[0]->
                                      children[0]->lexinfo,newFunc));
        }
        dumpToFile(file_sym, newFunc, curr->children[0]->children[0]);
        symbol_stack.push_back(nullptr);
        if (curr->children[1]->children.size() != 0){
           dumpParams(newFunc, curr->children[1]);
        }
        int funcSym;
        switch (curr->children[0]->symbol){
          case TOK_INT:
            funcSym = TOK_INTCON;
            break;
          case TOK_CHAR:
            funcSym = TOK_CHARCON;
            break;
          case TOK_STRING:
            funcSym = TOK_STRINGCON;
            break;
          case TOK_BOOL:
            funcSym = TOK_BOOL;
            break;
        }
        traverseFunc(curr->children[2], funcSym);
        blockcount--;
        symbol_stack.pop_back();
        break;
      }
      case TOK_STRUCT:
      {
        astree *temp = root->children[a];
        symbol *newSym = create_symbol(temp->children[0]);
        newSym->attributes.set(ATTR_struct);
        newSym->attributes.set(ATTR_typeid);
        newSym->type_id = *temp->children[0]->lexinfo;
        dumpToFile(file_sym,newSym,temp->children[0]);
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(temp->children[0]->lexinfo,newSym));
          symbol_stack.push_back(tab);
        }else{
          symbol_stack.back()->insert(symbol_entry(temp->children[0]->
                                                  lexinfo,newSym));
        }
        newSym->fields = new symbol_table();
        for(size_t i = 1; i<temp->children.size();i++){
          symbol *tempSym = create_symbol(temp->children[i]->
                                                children[0]);
          tempSym->attributes.set(ATTR_field);
          switch(temp->children[i]->symbol){
            case TOK_INT:
              tempSym->attributes.set(ATTR_int);
              break;
            case TOK_CHAR:
              tempSym->attributes.set(ATTR_char);
              break;
            case TOK_BOOL:
              tempSym->attributes.set(ATTR_bool);
              break;
            case TOK_STRING:
              tempSym->attributes.set(ATTR_string);
              break;
            case TOK_TYPEID:
              tempSym->attributes.set(ATTR_typeid);
              tempSym->attributes.set(ATTR_struct);
              break;
          }
          newSym->fields->insert(symbol_entry(temp->children[i]->
                                children[0]->lexinfo,tempSym));
        }
        dumpFields(newSym,temp);
        break;
      }
      case TOK_CALL:
        checkCall(curr);
        break;
      case TOK_PROTOTYPE:
      {
        symbol* newFunc = create_symbol(root->children[a]);
        blockcount++;
        newFunc->attributes.set(ATTR_function);
        switch(curr->children[0]->symbol){
          case TOK_INT:
            newFunc->attributes.set(ATTR_int);
            break;
          case TOK_STRING:
            newFunc->attributes.set(ATTR_string);
            break;
          case TOK_CHAR:
            newFunc->attributes.set(ATTR_char);
            break;
          case TOK_BOOL:
            newFunc->attributes.set(ATTR_bool);
            break;
          case TOK_TYPEID:
            newFunc->attributes.set(ATTR_struct);
            newFunc->attributes.set(ATTR_typeid);
            newFunc->type_id =*root->children[a]->children[0]->lexinfo;
            break;
        }
        newFunc->parameters = new vector<symbol*>;
        for(size_t i = 0; i<curr->children[1]->children.size(); i++){
          astree* param_node = curr->children[1]->children[i];
          symbol* param = create_symbol(param_node->children[0]);
          param->attributes.set(ATTR_variable);
          param->attributes.set(ATTR_lval);
          param->attributes.set(ATTR_param);
          int paramSym = param_node->symbol;
          switch(paramSym){
            case TOK_INT:
              param->attributes.set(ATTR_int);
              break;
            case TOK_CHAR:
              param->attributes.set(ATTR_char);
              break;
            case TOK_STRING:
              param->attributes.set(ATTR_string);
              break;
            case TOK_BOOL:
              param->attributes.set(ATTR_bool);
              break;
          }
          newFunc->parameters->push_back(param);
        }
        if(symbol_stack.back() == nullptr || symbol_stack.empty()){
          symbol_table* tab = new symbol_table();
          tab->insert(symbol_entry(curr->children[0]->children[0]->
                                                 lexinfo,newFunc));
          symbol_stack.pop_back();
          symbol_stack.push_back(tab);
        }else{
          symbol_stack.back()->insert(symbol_entry(curr->children[0]->
                                      children[0]->lexinfo,newFunc));
        }
        dumpToFile(file_sym, newFunc, curr->children[0]->children[0]);
        symbol_stack.push_back(nullptr);
        if (curr->children[1]->children.size() != 0){
           dumpParams(newFunc, curr->children[1]);
        }
        int funcSym;
        switch (curr->children[0]->symbol){
          case TOK_INT:
            funcSym = TOK_INTCON;
            break;
          case TOK_CHAR:
            funcSym = TOK_CHARCON;
            break;
          case TOK_STRING:
            funcSym = TOK_STRINGCON;
            break;
          case TOK_BOOL:
            funcSym = TOK_BOOL;
            break;
        }
        blockcount--;
        break;
      }
    }
  }
}

void traverseFunc(astree* root, int symbol){
  for (size_t a = 0; a < root->children.size(); ++a){
    int sym = root->children[a]->symbol;
    astree* curr = root->children[a];
    switch(sym){
      case TOK_VARDECL:
        travVardecl(curr);
        break;
      case TOK_IF: case TOK_WHILE:
         validCompare(curr);
         symbol_stack.push_back(nullptr);
         blockcount++;
         traverseAstree(curr->children[1]);
         blockcount--;
         symbol_stack.pop_back();
         break;
      case TOK_IFELSE:
        validCompare(curr);
        symbol_stack.push_back(nullptr);
        blockcount++;
        traverseAstree(curr->children[1]);
        blockcount--;
        symbol_stack.pop_back();
        symbol_stack.push_back(nullptr);
        blockcount++;
        traverseAstree(curr->children[2]);
        blockcount--;
        symbol_stack.pop_back();
        break;
      case TOK_RETURN:
        if( getReturnType(curr->children[0]) != symbol ){
          char *tname = (char* )get_yytname (symbol);
          if (strstr (tname, "TOK_") == tname) tname += 4;
          errprintf("%zu.%zu.%zu Return type does not match return"
          " type of "
          "function (%s)\n", curr->filenr, curr->linenr,
          curr->offset, tname);
        }
        break;
      case TOK_CALL:
        checkCall(curr);
        break;
    }
  }
}
