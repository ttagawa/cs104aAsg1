//John King joscking
//Tyler Tagawa ttagawa
#include "astree.h"
#include "oilgen.h"
#include "symtab.h"
#include "lyutils.h"

string getStrType(astree* curr){
  int sym = curr->symbol;
  if(curr->children.size()>1){
    switch(sym){
      case TOK_INT: return "int*";
      case TOK_CHAR: return "char*";
      case TOK_BOOL: return "char*";
      case TOK_STRING: return "char**";
      case TOK_TYPEID: const string *temp = curr->lexinfo;
        return "struct "+*temp+"**";
    }
  }
  switch(sym){
    case TOK_INT: return "int";
    case TOK_CHAR: return "char";
    case TOK_BOOL: return "char";
    case TOK_STRING: return "char*";
    case TOK_TYPEID: const string *temp = curr->lexinfo;
      return "struct "+*temp+"*";
  }
}

void dumpStruct(astree* root){
  for(size_t i=0;i<root->children.size();i++){
    int sym = root->children[i]->symbol;
    if(sym == TOK_STRUCT){
      const string* strval = root->children[i]->children[0]->lexinfo;
      symbol* structSym = lookupSym(strval);
      if(structSym!=NULL){
        fprintf(file_oil,"struct s_%s {\n",structSym->type_id.c_str());
        for(size_t j=1;j<root->children[i]->children.size();j++){
          astree* temp = root->children[i]->children[j];
          const string* field = temp->children[0]->lexinfo;
          if(temp->children.size()>1)
            field = temp->children[1]->lexinfo;
          string type = getStrType(temp);
          fprintf(file_oil, "\t%s f_%s_%s;\n", type.c_str(),strval->c_str(),
                                            field->c_str());
        }
        fprintf(file_oil,"}\n");
      }
    }
  }
}

void dumpStrincons(){
  for(size_t i=0;i<strvec.size();i++){
    fprintf(file_oil, "char* s%zu = %s;\n",i+1,strvec.at(i)->c_str());
  }
}

void dumpGlobalVars(astree* root){
  for(size_t i = 0;i<root->children.size();i++){
    astree* curr = root->children[i];
    int sym = curr->symbol;
    if(sym == TOK_VARDECL){
      fprintf(file_oil, "%s __%s;\n",
      getStrType(curr->children[0]).c_str(),
      curr->children[0]->children[0]->lexinfo->c_str() );
    }
  }
}

void dumpFunction(astree* root){
  for(size_t i = 0;i<root->children.size();i++){
    astree* curr = root->children[i];
    int sym = curr->symbol;
    if(sym == TOK_FUNCTION){
      fprintf(file_oil, "%s __%s (",
      getStrType(curr->children[0]).c_str(),
      curr->children[0]->children[0]->lexinfo->c_str());
      if(curr->children[1]->children.size()==0){
        fprintf(file_oil,")\n");
      }else{
        symbol* func = lookupSym(curr->children[0]->
                                children[0]->lexinfo);
        vector<symbol*> temp = *(func->parameters);
        for(size_t j = 0; j<temp.size();j++){
          symbol* tempSym = temp[j];
          string output = getStrType(curr->children[1]->children[j]);
          int blocknum = static_cast<int>(tempSym->block_nr);
          output+="_"+to_string(blocknum)+"_";
          output+=*(curr->children[1]->children[j]->
                                children[0]->lexinfo);
          if(j==temp.size()-1){
            fprintf(file_oil, "\n\t%s)\n",output.c_str());
          }else{
            fprintf(file_oil, "\n\t%s,",output.c_str());
          }
        }
      }
    }
  }
}

void makeOil(astree* root){
  dumpStruct(root);
  dumpStrincons();
  dumpGlobalVars(root);
  dumpFunction(root);
}
