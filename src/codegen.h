#ifndef CODEGEN_H
#define CODEGEN_H

#include "assem.h"
#include "frame.h"
#include "tree.h"
#include "util.h"

AS_instrList F_codegen(F_frame f, T_stmList stmList);

#endif