//John King joscking
//Tyler Tagawa ttagawa
#ifndef __OILGEN_H__
#define __OILGEN_H__

string getStrType(astree* curr);
void dumpStruct(astree* root);
void dumpStrincons();
void dumpGlobalVars(astree* root);
void makeOil(astree* root);

#endif
