//John King joscking
//Tyler Tagawa ttagawa
#include "astree.h"
#include "oilgen.h"
#include "symtab.h"
#include "lyutils.h"

int ireg = 1;
int breg = 1;

string getStrType(astree* curr){
  int sym = curr->symbol;
  if(curr->children.size()>1){
    switch(sym){
      case TOK_VOID: return "void";
      case TOK_INT: return "int*";
      case TOK_CHAR: return "char*";
      case TOK_BOOL: return "char*";
      case TOK_STRING: return "char**";
      case TOK_TYPEID: const string *temp = curr->lexinfo;
        return "struct "+*temp+"**";
    }
  }
  switch(sym){
    case TOK_VOID: return "void";
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
          fprintf(file_oil, "\t%s f_%s_%s;\n",
                            type.c_str(),strval->c_str(),
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

string recursBinop(astree* curr, astree* func, int flag){
  int sym = curr->symbol;
  switch(sym){
    case '+': case '-': case '/': case '*':{
      string str1 = recursBinop(curr->children[0], func, flag);
      string str2 = recursBinop(curr->children[1], func, flag);
      fprintf(file_oil,"\tint i%d = %s %s %s;\n",ireg,str1.c_str(),
                      curr->lexinfo->c_str(),str2.c_str());
      ireg++;
      return "i" + to_string(ireg-1);
    }
    case TOK_IDENT:{
      symbol* temp = lookupSym(curr->lexinfo);
      int blockNum = 1;
      if(temp != NULL){
        blockNum = static_cast<int>(temp->block_nr);
      }else{
        if( func != NULL){
          for(size_t i = 0; i < func->children[1]
                                      ->children.size(); i++){
            if(*(curr->lexinfo) ==
              *(func->children[1]->children[i]->children[0]->lexinfo)){
                blockNum = 1;
            }
          }
        }
      }
      if ( flag == 0){
        return "__"+*(curr->lexinfo);
      }else{
        return "_"+to_string(blockNum)+"_"+*(curr->lexinfo);
      }


    }
    case TOK_INTCON:{
      string temp = *(curr->lexinfo);
      return temp;
    }
    case TOK_LE: case TOK_GE: case TOK_LT: case TOK_GT: case TOK_EQ:
    case TOK_NE:{
      string str1 = recursBinop(curr->children[0], NULL, flag);
      string str2 = recursBinop(curr->children[1], NULL, flag);
      return str1 +" "+ *(curr->lexinfo) +" "+str2;
      break;
    }
    default:
      return "";
      break;
  }
}

void printVardecl(astree* curr, astree* func, int flag){
  string result = "";
  result = recursBinop(curr->children[1], func, flag);
  string left = "";
  if(curr->children[0]->symbol == TOK_IDENT){
    symbol* temp = lookupSym(curr->children[0]->lexinfo);
    if(func != NULL){
      for(size_t i = 0; i < func->children[1]->children.size(); i++){
        if(*(curr->children[0]->lexinfo) ==
          *(func->children[1]->children[i]->children[0]->lexinfo)){
            int blockNum = 1;
            left+="_"+to_string(blockNum)+"_";
            left+=*(curr->children[0]->children[0]->lexinfo);
        }
      }
    }
    if(left == ""){
      int blockNum = static_cast<int>(curr->block);
      if(flag == 0){
          left+="__";
      }else{
        left+="_"+to_string(blockNum)+"_";
      }
      left+=*(curr->children[0]->lexinfo);
    }
  }else{
    int blockNum = static_cast<int>(curr->block);
    if(flag == 0){
          left+="int __";
    }else{
          left+="int _"+to_string(blockNum)+"_";
    }
    left+=*(curr->children[0]->children[0]->lexinfo);
  }
  fprintf(file_oil,"\t%s = %s;\n",left.c_str(),result.c_str());
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
      fprintf(file_oil, "{\n");
      for(size_t k = 0;k<curr->children[2]->children.size();k++){
        int blockSym = curr->children[2]->children[k]->symbol;
        switch(blockSym){
          case TOK_VARDECL: case '=':
            printVardecl(curr->children[2]->children[k], curr, 1);
            break;
          case TOK_RETURN:{
            string result = "";
            if(curr->children[2]->children[k]->children.size() > 1){
              result = recursBinop(curr->children[2]->children[k]
                                          ->children[1], curr, 1);
            }else{
              result = recursBinop(curr->children[2]->children[k]
                                        ->children[0], curr, 1);
            }
            fprintf(file_oil, "\treturn %s\n", result.c_str());
            break;
          }
          case TOK_WHILE:
          {
            dumpWhile(curr->children[2]->children[k], 1);
            break;
          }
          case TOK_IF:
            dumpIf(curr, 1);
            break;
          case TOK_IFELSE:
            dumpIfElse(curr, 1);
            break;
          case TOK_CALL:
            dumpCall(curr);
          default:
            break;
        }
      }
    }
  }
  fprintf(file_oil, "}\n");
}

void dumpExpressions(astree* root, int flag){
  for(size_t i = 0; i < root->children.size(); i++){
    int sym = root->children[i]->symbol;
    astree* curr = root->children[i];
    switch(sym){
      case TOK_WHILE:{
        dumpWhile(curr, flag);
        break;
      }
      case TOK_VARDECL: case '=':
        printVardecl(curr, NULL, flag);
        break;
      case TOK_CALL:
        dumpCall(curr);
        break;
      case TOK_IF:
        dumpIf(curr, flag);
        break;
      case TOK_IFELSE:
        dumpIfElse(curr, flag);
        break;
        default:
        break;
    }
  }
}

void dumpCall(astree* curr){
  if(curr->children.size() > 1){

  }else{
    fprintf(file_oil,"\t__%s();\n",curr->children[0]
                                    ->lexinfo->c_str());
  }
}

void dumpWhile(astree* curr, int flag){
  fprintf(file_oil, "while_%zu_%zu_%zu:;\n", curr->filenr,
                    curr->linenr, curr->offset);
  string comp = recursBinop(curr->children[0], NULL, flag);
  fprintf(file_oil, "\tchar b%d = %s;\n", breg, comp.c_str());
  fprintf(file_oil, "\tif (!b%d) goto break_%zu_%zu_%zu;\n",
                    breg, curr->filenr,
                    curr->linenr, curr->offset);
  breg++;
  dumpExpressions(curr->children[1], flag);
  fprintf(file_oil, "\tgoto while_%zu_%zu_%zu;\n", curr->filenr,
                    curr->linenr, curr->offset);
  fprintf(file_oil, "break_%zu_%zu_%zu:\n", curr->filenr,
                    curr->linenr, curr->offset);
}

void dumpIf(astree* curr, int flag){
  fprintf(file_oil, "if_%zu_%zu_%zu:;\n", curr->filenr,
                    curr->linenr, curr->offset);
  string comp = recursBinop(curr->children[0], NULL, flag);
  fprintf(file_oil, "\tchar b%d = %s;\n", breg, comp.c_str());
  breg++;
  fprintf(file_oil, "\tif (!b%d) goto fi_%zu_%zu_%zu;\n",
                    breg, curr->filenr,
                    curr->linenr, curr->offset);
  dumpExpressions(curr->children[1], flag);
  fprintf(file_oil, "fi_%zu_%zu_%zu:;\n", curr->filenr,
                    curr->linenr, curr->offset);
}
void dumpIfElse(astree* curr, int flag){
  fprintf(file_oil, "if_%zu_%zu_%zu:;\n", curr->filenr,
                    curr->linenr, curr->offset);
  string comp = recursBinop(curr->children[0], NULL, flag);
  fprintf(file_oil, "\tchar b%d = %s;\n", breg, comp.c_str());
          breg++;
  fprintf(file_oil, "\tif (!b%d) goto else_%zu_%zu_%zu;\n",
                    breg, curr->filenr,
                    curr->linenr, curr->offset);
  dumpExpressions(curr->children[1], flag);
  fprintf(file_oil, "\tgoto fi_%zu_%zu_%zu;\n",
                    curr->filenr,
                    curr->linenr, curr->offset);
fprintf(file_oil, "else_%zu_%zu_%zu:;\n",
        curr->filenr,
        curr->linenr, curr->offset);
  dumpExpressions(curr->children[2], flag);
  fprintf(file_oil, "fi_%zu_%zu_%zu:;\n", curr->filenr,
                    curr->linenr, curr->offset);
}

void makeOil(astree* root){
  dumpStruct(root);
  dumpStrincons();
  dumpGlobalVars(root);
  dumpFunction(root);
  fprintf(file_oil, "void __ocmain (void)\n{\n");
  dumpExpressions(root, 0);
  fprintf(file_oil, "}");
}
