#include <stdio.h>
#include <stdlib.h>

#include "absyn.h"
#include "errormsg.h"
#include "frame.h"
#include "parse.h"
#include "semant.h"
#include "symbol.h"
#include "util.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: a.out filename\n");
    exit(1);
  }
  A_exp absyn_tree_root = parse(argv[1]);
  if (absyn_tree_root) {
    F_fragList fl = SEM_transProg(absyn_tree_root);
  } else {
    fprintf(stderr, "parsing failed!\n");
    return 1;
  }
  return 0;
}