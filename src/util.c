#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *checked_malloc(int len) {
  void *p = malloc(len);
  if (!p) {
    fprintf(stderr, "\nRan out of memory!\n");
    exit(1);
  }
  return p;
}

string String(char *s) {
  string p = checked_malloc(strlen(s) + 1);
  strcpy(p, s);
  return p;
}

U_boolList U_BoolList(bool head, U_boolList tail) {
  U_boolList list = checked_malloc(sizeof(*list));
  list->head = head;
  list->tail = tail;
  return list;
}

string FormatString(string s, ...) {
  va_list ap;
  char tmp[50];
  va_start(ap, s);
  int len = vsnprintf(tmp, 50, s, ap);
  va_end(ap);
  string res = checked_malloc(len + 1);
  strcpy(res, tmp);
  return res;
}