//John King joscking
//Tyler Tagawa ttagawa
#include "oilgen.h"
#include "symtab.h"

void dumpStruct(astree* root){
  for(size_t i=0;i<root->children.size();i++){
    int sym = root->children[i]->symbol;
    if(sym == TOK_STRUCT){
      const string* strval = root->children[i]->children[0]->lexinfo;
      symbol* structSym = lookupSym(strval);
      if(structSym!=NULL){
        fprintf(file_oil,"s_%s\n{\n",structSym->type_id.c_str());
        for(size_t j=1;j<root->children[i]->children.size();j++){
          astree* temp = root->children[i]->children[j]->children[0];
          const string* field = temp->lexinfo;
          fprintf(file_oil, "\tf_%s_%s;\n", strval->c_str(),
                                            field->c_str());
        }
        fprintf(file_oil,"}\n");
      }
    }
  }
}

void makeOil(astree* root){
  dumpStruct(root);
}
