#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "absyn.h"
#include "assem.h"
#include "canon.h"
#include "codegen.h"
#include "errormsg.h"
#include "escape.h"
#include "frame.h"
#include "parse.h"
#include "prabsyn.h"
#include "printtree.h"
#include "semant.h"
#include "symbol.h"
#include "temp.h"
#include "translate.h"
#include "tree.h"
#include "util.h"

int print_absyn_tree = 0, print_ir_tree = 0, print_before_reg_alloc = 0,
    print_after_reg_alloc = 0, print_canon = 0;

static void do_proc(FILE *out, F_frame frame, T_stm body) {
  AS_instrList instr_l = NULL;
  T_stmList stm_l = NULL;

  stm_l = C_linearize(body);
  stm_l = C_traceSchedule(C_basicBlocks(stm_l));
  if (print_canon) {
    printStmList(out, stm_l);
  }
  
  instr_l = F_codegen(frame, stm_l);
  if (print_before_reg_alloc) {
    AS_printInstrList(out, instr_l, F_tempMap());
  }
}

static void do_string(FILE *out, F_frame frame, T_stm body) {}

void help_menu(const char *prog_name) {
  printf("Usage: %s [flags]\n", prog_name);
  printf("-h    prints this usage guide\n");
  printf("-p [path]   sets the input file path\n");
  printf("-o    sets the name of the output binary\n");
  printf("-a    prints the abstract syntax tree\n");
  printf("-i    prints the intermediate representation\n");
  printf("-c    prints the canonical intermediate representation tree\n");
  printf("-s    prints the generated assembly code before regs allocation\n");
}

int main(int argc, char *argv[]) {
  int ch;
  char *input_file, *output_file;
  while ((ch = getopt(argc, argv, "aho:p:isSc")) != -1) {
    switch (ch) {
      case 'p':
        input_file = (char *)malloc(strlen(optarg));
        strcpy(input_file, optarg);
        break;

      case 'a':
        print_absyn_tree = 1;
        break;

      case 'o':
        output_file = (char *)malloc(strlen(optarg));
        strcpy(output_file, optarg);
        break;

      case 'i':
        print_ir_tree = 1;
        break;

      case 'c':
        print_canon = 1;
        break;

      case 's':
        print_before_reg_alloc = 1;
        break;

      case 'S':
        print_after_reg_alloc = 1;
        break;

      case 'h':
        input_file ? help_menu(input_file) : help_menu("./tc");
        exit(EXIT_SUCCESS);
        break;

      default:
        help_menu(input_file);
        exit(EXIT_FAILURE);
        break;
    }
  }

  A_exp absyn_root = parse(input_file);
  if (absyn_root == NULL) {
    fprintf(stderr, "cannot parse file\n");
    exit(1);
  }

  FILE *out = stdout;
  if (print_absyn_tree) {
    fprintf(out, "========== Absyn Tree ==========\n");
    pr_exp(out, absyn_root, 0);
    fprintf(out, "\n========== End ==========\n\n");
  }

  Esc_findEscape(absyn_root);

  F_fragList frags = SEM_transProg(absyn_root);
  if (EM_anyErrors) {
    exit(1);
  }
  F_fragList string_frags = frags;

  if (print_ir_tree) {
    fprintf(out, "========== IR Tree ==========\n");
    Tr_printTree(get_exp(absyn_root));
    fprintf(out, "\n========== End ==========\n\n");
  }
  if (print_before_reg_alloc) {
    fprintf(out, "========== STRING LABELS ==========\n");
    for (; string_frags; string_frags = string_frags->tail)
      if (string_frags->head->kind == F_stringFrag)
        fprintf(out, "%s: %s\n\n", Temp_labelstring(Temp_newlabel()),
                string_frags->head->u.stringg.str);
    fprintf(out, "========== END STRING LABELS ==========\n");
  }
  if (print_before_reg_alloc || print_canon) {
    for (; frags; frags = frags->tail)
      if (frags->head->kind == F_procFrag)
        do_proc(out, frags->head->u.proc.frame, frags->head->u.proc.body);
  }
  fclose(out);

  return 0;
}