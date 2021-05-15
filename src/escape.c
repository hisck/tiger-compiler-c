#include "escape.h"

#include "symbol.h"

typedef struct E_escentry_ *E_escentry;

struct E_escentry_ {
  bool *escape;
  int depth;
};

E_escentry EscEntry(bool *escape, int depth) {
  E_escentry eet = checked_malloc(sizeof(*eet));

  eet->escape = escape;
  eet->depth = depth;

  return eet;
}

void Esc_findEscape(A_exp exp);

static void traverseExp(S_table env, int depth, A_exp e);
static void traverseDec(S_table env, int depth, A_dec d);
static void traverseVar(S_table env, int depth, A_var v);

void Esc_findEscape(A_exp exp) { traverseExp(S_empty(), 0, exp); }

static void traverseExp(S_table env, int depth, A_exp e) {
  switch (e->kind) {
    case A_varExp:
      traverseVar(env, depth, e->u.var);
      return;

    case A_callExp: {
      A_expList el = e->u.call.args;
      for (; el; el = el->tail) {
        traverseExp(env, depth, el->head);
      }
      return;
    }

    case A_opExp:
      traverseExp(env, depth, e->u.op.left);
      traverseExp(env, depth, e->u.op.right);
      return;

    case A_seqExp: {
      A_expList el = e->u.seq;
      for (; el; el = el->tail) {
        traverseExp(env, depth, el->head);
      }
      return;
    }

    case A_assignExp:
      traverseVar(env, depth, e->u.assign.var);
      traverseExp(env, depth, e->u.assign.exp);
      return;

    case A_ifExp:
      traverseExp(env, depth, e->u.iff.test);
      traverseExp(env, depth, e->u.iff.then);
      if (e->u.iff.elsee != NULL) {
        traverseExp(env, depth, e->u.iff.elsee);
      }
      return;

    case A_whileExp:
      traverseExp(env, depth, e->u.whilee.test);
      traverseExp(env, depth, e->u.whilee.body);
      return;

    case A_doWhileExp:
      traverseExp(env, depth, e->u.dowhilee.body);
      traverseExp(env, depth, e->u.dowhilee.test);
      return;

    case A_forExp:
      S_enter(env, e->u.forr.var, EscEntry(&(e->u.forr.escape), depth));
      traverseExp(env, depth, e->u.forr.body);
      return;

    case A_letExp: {
      A_decList d;
      S_beginScope(env);

      for (d = e->u.let.decs; d; d = d->tail) {
        traverseDec(env, depth, d->head);
      }
      traverseExp(env, depth, e->u.let.body);
      S_endScope(env);
      return;
    }

    case A_arrayExp:
      traverseExp(env, depth, e->u.array.init);
      traverseExp(env, depth, e->u.array.size);
      return;

    // other cases, do nothing
    default:
      return;
  }

  assert(0);
}

static void traverseDec(S_table env, int depth, A_dec d) {
  switch (d->kind) {
    case A_varDec:
      traverseExp(env, depth, d->u.var.init);

      d->u.var.escape = 0;
      S_enter(env, d->u.var.var, EscEntry(&(d->u.var.escape), depth));

      return;

    case A_functionDec: {
      A_fundec f;
      for (A_fundecList fl = d->u.function; fl; fl = fl->tail) {
        f = fl->head;
        S_beginScope(env);

        for (A_fieldList l = f->params; l; l = l->tail) {
          l->head->escape = 1;
          S_enter(env, l->head->name, EscEntry(&(l->head->escape), depth));
        }

        traverseExp(env, depth + 1, f->body);
        S_endScope(env);
      }
      return;
    }

    case A_typeDec:
      return;
  }

  assert(0);
}

static void traverseVar(S_table env, int depth, A_var v) {
  switch (v->kind) {
    case A_simpleVar: {
      E_escentry eet = S_look(env, v->u.simple);

      if (eet && depth > eet->depth) {
        *(eet->escape) = 1;
      }

      return;
    }

    case A_fieldVar:
      traverseVar(env, depth, v->u.field.var);

      return;

    case A_subscriptVar:
      traverseVar(env, depth, v->u.subscript.var);
      traverseExp(env, depth, v->u.subscript.exp);

      return;
  }

  assert(0);
}