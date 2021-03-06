#include <stdio.h>
#include <stdlib.h>

#include "errormsg.h"
#include "parse.h"
#include "util.h"

extern int yyparse(void);
extern A_exp absyn_root;

void parseT(string fname) {
  EM_reset(fname);
  if (yyparse() == 0) /* parsing worked */
    fprintf(stderr, "Parsing successful!\n");
  else
    fprintf(stderr, "Parsing failed\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: a.out filename\n");
    exit(1);
  }
  parseT(argv[1]);
  return 0;
}