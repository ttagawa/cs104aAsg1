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
  char c;
  //string s = NULL;
  int argi=1;
while((c = getopt (argc,argv,"ly@:D:"))!=-1){
  argi++;
  switch(c){
    case 'l':printf("lex debugging on");
             yy_flex_debug=1;
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
      FILE* pipe = popen (command.c_str(), "r");
      if (pipe == NULL) {
         syserrprintf (command.c_str());
      }else {
         cpplines (pipe, filename);
         int pclose_rc = pclose (pipe);
         eprint_status (command.c_str(), pclose_rc);
      }
   filename  = basename(filename);
   string filename1(filename);
   size_t pos = filename1.find('.');
   string base = filename1.substr(0,pos);
   string suffix = filename1.substr(pos);
   if(suffix.compare(".oc")!=0){
     errprintf("please enter a .oc file\n");
     return get_exitstatus();
   }
   char* final = new char[base.length() + 5];
   strcpy(final,base.c_str());
   strcat(final,".str");
   ofstream file(final);
   dump_stringset(file);
   file.close();
   delete [] final;
   return get_exitstatus();
}
