//John King joscking
//Tyler Tagawa ttagawa
#ifndef __OILGEN_H__
#define __OILGEN_H__

string getStrType(astree* curr);
void dumpStruct(astree* root);
void dumpStrincons();
void dumpGlobalVars(astree* root);
string recursBinop(astree* curr, astree *func, int flag);
void printVardecl(astree* curr, astree* func, int flag);
void dumpFunction(astree* root);
void dumpExpressions(astree* root, int flag);
void dumpWhile(astree* root, int flag);
void dumpIf(astree* root, int flag);
void dumpCall(astree* curr);
void dumpIfElse(astree* root, int flag);
void makeOil(astree* root);

#endif
