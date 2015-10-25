// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

#include <string>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <fstream>
#include <unistd.h>
#include <getopt.h>
#include <iostream>


#include "auxlib.h"
#include "stringset.h"
#include "lyutils.h"
FILE* file_tok;
string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;
// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}
// Run cpp against the lines of the file.
void cpplines (FILE* pipe, char* filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, filename);
      if (sscanf_rc == 2) {
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         intern_stringset(token);
      }
      ++linenr;
   }
}

int main (int argc, char** argv) {
  yy_flex_debug=0;
  yydebug=0;
  char c;
  //string s = NULL;
  int argi=1;
while((c = getopt (argc,argv,"ly@:D:"))!=-1){
  argi++;
  switch(c){
    case 'l':yy_flex_debug=1;
             break;
    case 'y':printf("yak debugging on");
            //yydebug =1;
            break;
    case '@':set_debugflags(optarg);break;
    case 'D':CPP+=" -D"+string(optarg);break;
    case '?':errprintf("Not proper option");
             return get_exitstatus();
    default:break;
  }
}

   set_execname (argv[0]);
  char* filename = NULL;
  filename = argv[argi];
  string command = CPP + " " + filename;
  filename  = basename(filename);
  string filename1(filename);
  size_t pos = filename1.find('.');
  string base = filename1.substr(0,pos);
  string suffix = filename1.substr(pos);
  char* final_str = new char[base.length() + 5];
  char* final_tok = new char[base.length() + 5];
  strcpy(final_str,base.c_str());
  strcat(final_str,".str");
  strcpy(final_tok,base.c_str());
  strcat(final_tok,".tok");
  ofstream file_str(final_str);
    //  ofstream file_tok(final_tok);
      yyin = popen (command.c_str(), "r");
      if (yyin == NULL) {
         syserrprintf (command.c_str());
      }else {
        file_tok = fopen(final_tok, "w");
      //  int tCount=0;
        while(yylex()!= YYEOF){
          //  yyprint(file_tok,tCount,yylval);
        }
        fclose(file_tok);
      }
   if(suffix.compare(".oc")!=0){
     errprintf("please enter a .oc file\n");
     return get_exitstatus();
   }
   dump_stringset(file_str);
   file_str.close();
  // file_tok.close();
   delete [] final_str;
   delete [] final_tok;
   yylex_destroy();
   return get_exitstatus();
}
