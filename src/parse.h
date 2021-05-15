#ifndef PARSE_H__
#define PARSE_H__

#include <stdio.h>

#include "absyn.h"
#include "errormsg.h"
#include "symbol.h"
#include "util.h"

/* function prototype from parse.c */
A_exp parse(string fname);

#endif