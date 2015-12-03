//John King joscking
//Tyler Tagawa ttagawa 
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"


astree::astree (int symbol, int filenr, int linenr,
                int offset, const char* clexinfo):
        symbol (symbol), filenr (filenr), linenr (linenr),
        offset (offset), lexinfo (intern_stringset (clexinfo)) {
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           (void*) this, filenr, linenr, offset,
           get_yytname (symbol), lexinfo->c_str());
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt3 (astree* root, astree* c1, astree* c2, astree* c3){
  adopt1(root,c1);
  adopt1(root,c2);
  adopt1(root,c3);
  return root;
}

astree* appendAdopt(astree* parent, astree* child){
  parent->children.insert(parent->children.end(),
  child->children.begin(),child->children.end());
  return parent;
}

astree* funcCheck (astree* root, astree* param, astree *child){
  astree* node;
  if(child->symbol == (int)';'){
    node = new astree(TOK_PROTOTYPE,root->filenr,
      root->linenr,root->offset,"");
    adopt2(node,root,param);
  }else{
    node = new astree (TOK_FUNCTION,root->filenr,
      root->linenr,root->offset,"");
    adopt3(node,root,param,child);
  }
  return node;
}

static void dump_node (FILE* outfile, astree* node) {
  // fprintf (outfile, "%p->{%s(%d) %ld:%ld.%03ld \"%s\" [",
  //          node, get_yytname (node->symbol), node->symbol,
  //          node->filenr, node->linenr, node->offset,
  //          node->lexinfo->c_str());
  fprintf (outfile, "%ld %ld.%03ld %3d %-15s (%s) \n",
            node->filenr, node->linenr, node->offset,
            node->symbol, get_yytname (node->symbol),
            node->lexinfo->c_str());
   bool need_space = false;
   for (size_t child = 0; child < node->children.size();
        ++child) {
      if (need_space) fprintf (outfile, " ");
      need_space = true;
      fprintf (outfile, "%p", node->children.at(child));
   }
//   fprintf (outfile, "]}\n");
}

string getAttrs(astree* root){
  string result = "";
  int sym = root->symbol;
  switch(sym){
    case TOK_VOID:
      root->attribs.set(ATTR_void);
      result += "void";
      break;
    case TOK_BOOL:
      root->attribs.set(ATTR_bool);
      result += "bool";
      break;
    case TOK_INT:
      root->attribs.set(ATTR_int);
      result += "int";
      break;
    case TOK_CHAR:
      root->attribs.set(ATTR_char);
      result += "char";
      break;
    case TOK_STRING:
      root->attribs.set(ATTR_string);
      result += "string";
      break;
    case TOK_ARRAY:
        root->attribs.set(ATTR_array);
        result += "array";
        break;
    case TOK_FUNCTION:
      root->attribs.set(ATTR_function);
      result += "function";
      break;
    case TOK_FIELD:
      root->attribs.set(ATTR_field);
      result += "field";
      break;
    case TOK_TYPEID:
      root->attribs.set(ATTR_typeid);
      result += "typeid";
      break;
    case TOK_DECLID:
      root->attribs.set(ATTR_variable);
      root->attribs.set(ATTR_lval);
      result += "variable lval";
      break;
    case TOK_PARAM:
      root->attribs.set(ATTR_param);
      result += "param";
      break;
    case TOK_IDENT:
      root->attribs.set(ATTR_variable);
      result += "variable";
      break;
    case '+': case '*': case '/': case '%': case '-': case TOK_ORD:
      root->attribs.set(ATTR_int);
      root->attribs.set(ATTR_vreg);
      result += "int vreg";
      break;
    case TOK_CHR:
      root->attribs.set(ATTR_char);
      root->attribs.set(ATTR_vreg);
      result += "char vreg";
      break;
    case TOK_INTCON:
      root->attribs.set(ATTR_int);
      root->attribs.set(ATTR_const);
      result += "int const";
      break;
    case TOK_CHARCON:
      root->attribs.set(ATTR_char);
      root->attribs.set(ATTR_const);
      result += "char const";
      break;
    case TOK_STRINGCON:
      root->attribs.set(ATTR_string);
      root->attribs.set(ATTR_const);
      result += "string const";
      break;
    case TOK_FALSE: case TOK_TRUE:
      root->attribs.set(ATTR_bool);
      root->attribs.set(ATTR_const);
      result += "bool const";
      break;
    case TOK_NULL:
      root->attribs.set(ATTR_null);
      root->attribs.set(ATTR_const);
      result += "null const";
      break;
    case TOK_GE: case TOK_LT: case TOK_GT: case TOK_LE:
      root->attribs.set(ATTR_bool);
      root->attribs.set(ATTR_vreg);
      result += "bool vreg";
      break;
    case TOK_INDEX:
      root->attribs.set(ATTR_lval);
      root->attribs.set(ATTR_vaddr);
      result += "vaddr lval";
      break;
    case '.':
      root->attribs.set(ATTR_lval);
      root->attribs.set(ATTR_vaddr);
      result += "vaddr lval";
      break;
    case TOK_NEWSTRING:
      root->attribs.set(ATTR_string);
      root->attribs.set(ATTR_vreg);
      result += "string vreg";
      break;
    case TOK_NEWARRAY:
      root->attribs.set(ATTR_vreg);
      root->attribs.set(ATTR_array);
      result+= "array vreg";
      break;
    case TOK_POS: case TOK_NEG:
      root->attribs.set(ATTR_vreg);
      root->attribs.set(ATTR_int);
      result += "int vreg";
      break;
  }
  return result;
}

static void dump_astree_rec (FILE* outfile, astree* root,
                             int depth) {
   if (root == NULL) return;
   char *tname = (char*)get_yytname(root->symbol);
   if(strstr(tname,"TOK_")==tname)tname+=4;
    string bars;
    for(int i=0;i<depth;i++){
      bars+="|   ";
    }
    string attrs = getAttrs(root);
    fprintf(outfile,"%s%s \"%s\" %zu.%zu.%zu {%d} %s",bars.c_str(),
            tname,root->lexinfo->c_str(),
            root->filenr,root->linenr,root->offset,
            depth, attrs.c_str());
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size();
        ++child) {
      dump_astree_rec (outfile, root->children[child],
                       depth + 1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep) {
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n",
               get_yytname (toknum), toknum);
   }
   fflush (NULL);
}


void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%p]-> %d:%d.%d: %s: \"%s\")\n",
           root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

//RCSC("$Id: astree.cpp,v 1.6 2015-04-09 19:31:47-07 - - $")
