
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

static void dump_astree_rec (FILE* outfile, astree* root,
                             int depth) {
   if (root == NULL) return;
   char *tname = (char*)get_yytname(root->symbol);
   if(strstr(tname,"TOK_")==tname)tname+=4;
    string bars;
    for(int i=0;i<depth;i++){
      bars+="|   ";
    }
    fprintf(outfile,"%s%s \"%s\" %zu.%zu.%zu",bars.c_str(),
            tname,root->lexinfo->c_str(),
            root->filenr,root->linenr,root->offset);
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
