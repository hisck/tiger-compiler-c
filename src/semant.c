#include "semant.h"

typedef struct expty_ {
  Tr_exp exp;
  Ty_ty ty;
} expty;

static expty expTy(Tr_exp exp, Ty_ty ty) {
  expty e;
  e.exp = exp;
  e.ty = ty;
  return e;
}

static expty transVar(Tr_level level, S_table venv, S_table tenv, A_var v);
static expty transExp(Tr_level level, S_table venv, S_table tenv, A_exp e);
static Tr_exp transDec(Tr_level level, S_table venv, S_table tenv, A_dec d);
static Ty_ty transTy(S_table tenv, A_ty t);

// inside flag (for loop, while loop)
static int inside = 0;
static Tr_exp brk[16];  // MAXIMUM LOOP NEST 15

// Turn 'Ty_name' to actual type
static Ty_ty actual_ty(Ty_ty ty) {
  Ty_ty t = ty;
  while (t->kind == Ty_name) t = t->u.name.ty;
  return t;
}

// Compare two actual type
// Ty_nil == Ty_record
// Ty_record or Ty_array: compare the reference
static bool actual_eq(Ty_ty source, Ty_ty target) {
  Ty_ty t1 = actual_ty(source);
  Ty_ty t2 = actual_ty(target);
  return ((t1->kind == Ty_record || t1->kind == Ty_array) && t1 == t2) ||
         (t1->kind == Ty_record && t2->kind == Ty_nil) ||
         (t1->kind == Ty_nil && t2->kind == Ty_record) ||
         (t1->kind != Ty_record && t1->kind != Ty_array &&
          t1->kind == t2->kind);
}

F_fragList SEM_transProg(A_exp exp) {
  S_table venv = E_base_venv(), tenv = E_base_tenv();
  expty trans_exp = transExp(Tr_outermost(), venv, tenv, exp);
  // Tr_printTree(trans_exp.exp);  // print the ir tree
  return Tr_getResult();
}

Tr_exp get_exp(A_exp exp) {
  S_table venv = E_base_venv(), tenv = E_base_tenv();
  expty trans_exp = transExp(Tr_outermost(), venv, tenv, exp);
  return trans_exp.exp;
}

/**
 * translate abstract variable expression to ir structure
 */
static expty transVar(Tr_level level, S_table venv, S_table tenv, A_var v) {
  switch (v->kind) {
    case A_simpleVar: {
      E_enventry x = S_look(venv, v->u.simple);
      if (x && x->kind == E_varEntry) {
        return expTy(Tr_simpleVar(x->u.var.access, level),
                     actual_ty(x->u.var.ty));
      } else {
        EM_error(v->pos, "simple var expression: undefined variable %s",
                 S_name(v->u.simple));
        exit(1);
      }
    }
    case A_fieldVar: {
      expty var = transVar(level, venv, tenv, v->u.field.var);
      if (var.ty->kind != Ty_record) {
        EM_error(v->u.field.var->pos,
                 "field var expression: not a record type variable");
        exit(1);
      } else {
        Ty_fieldList fl = NULL;
        int offset = 0;
        for (fl = var.ty->u.record; fl; fl = fl->tail, offset++) {
          if (fl->head->name == v->u.field.sym) {
            return expTy(Tr_fieldVar(var.exp, offset), actual_ty(fl->head->ty));
          }
        }
        EM_error(v->u.field.var->pos,
                 "field var expression: no such field <%s> in the record",
                 S_name(v->u.field.sym));
        exit(1);
      }
    }
    case A_subscriptVar: {
      expty var = transVar(level, venv, tenv, v->u.subscript.var);
      if (var.ty->kind != Ty_array) {
        EM_error(v->u.subscript.var->pos,
                 "subscript var expression: not an array type variable");
        exit(1);
      } else {
        expty index = transExp(level, venv, tenv, v->u.subscript.exp);
        if (index.ty->kind != Ty_int) {
          EM_error(v->u.subscript.exp->pos,
                   "subscript var expression: integer required in array index");
          exit(1);
        }
        return expTy(Tr_subscriptVar(var.exp, index.exp),
                     actual_ty(var.ty->u.array));
      }
    }
  }
  assert(0);  // wrong kind
}

/**
 * translate abstract expression to ir expression
 */
static expty transExp(Tr_level level, S_table venv, S_table tenv, A_exp e) {
  switch (e->kind) {
    case A_varExp:
      return transVar(level, venv, tenv, e->u.var);
    case A_nilExp:
      return expTy(Tr_nilExp(), Ty_Nil());
    case A_intExp:
      return expTy(Tr_intExp(e->u.intt), Ty_Int());
    case A_stringExp:
      return expTy(Tr_stringExp(e->u.stringg), Ty_String());
    case A_callExp: {
      E_enventry fun_entry = S_look(venv, e->u.call.func);
      if (!fun_entry || (fun_entry->kind != E_funEntry)) {
        EM_error(e->pos, "call expression: undefined type %s",
                 S_name(e->u.call.func));
        exit(1);
      } else {
        A_expList el = NULL;
        Ty_tyList tl = NULL;
        Tr_expList tr_el = NULL;
        for (el = e->u.call.args, tl = fun_entry->u.fun.formals; el && tl;
             el = el->tail, tl = tl->tail) {
          expty exp = transExp(level, venv, tenv, el->head);
          Ty_ty actual = actual_ty(tl->head);
          if (!actual_eq(tl->head, exp.ty)) {
            EM_error(
                el->head->pos,
                "call expression: argument type dosen't match the paramater");
            exit(1);
          }
          tr_el = Tr_ExpList(exp.exp, tr_el);
        }
        if (el) {
          EM_error(el->head->pos, "call expression: too many arguments");
          exit(1);
        }
        if (tl) {
          EM_error(e->pos, "call expression: not enough arguments");
          exit(1);
        }
        return expTy(Tr_callExp(level, fun_entry->u.fun.level,
                                fun_entry->u.fun.label, tr_el),
                     actual_ty(fun_entry->u.fun.results));
      }
    } /* callexp */
    case A_opExp: {
      expty left = transExp(level, venv, tenv, e->u.op.left);
      expty right = transExp(level, venv, tenv, e->u.op.right);
      switch (e->u.op.oper) {
        case A_plusOp:
        case A_minusOp:
        case A_timesOp:
        case A_divideOp: {
          // operand must be integer, return value must be integer
          if (left.ty->kind != Ty_int)
            EM_error(e->u.op.left->pos, "binary operation: integer required");
          if (right.ty->kind != Ty_int)
            EM_error(e->u.op.right->pos, "binary operation: integer required");
          return expTy(Tr_arithExp(e->u.op.oper, left.exp, right.exp),
                       Ty_Int());
        }
        case A_eqOp:
        case A_neqOp: {
          // can be applied to integers, strings, and two arrays or records
          // be careful about records, because nil is also record-type
          if (!actual_eq(left.ty, right.ty)) {
            EM_error(e->pos, "operators compare: different type for compare");
            return expTy(Tr_noExp(), Ty_Int());
          }
          return expTy(Tr_relExp(e->u.op.oper, left.exp, right.exp), Ty_Int());
        }
        case A_ltOp:
        case A_leOp:
        case A_gtOp:
        case A_geOp: {
          if (left.ty->kind != Ty_int || right.ty->kind != Ty_int) {
            EM_error(e->pos, "binary compare: integer required");
            return expTy(Tr_noExp(), Ty_Int());
          }
          return expTy(Tr_relExp(e->u.op.oper, left.exp, right.exp), Ty_Int());
        }
      } /* switch */
    }   /* A_opExp */
    case A_recordExp: {
      Ty_ty record_typ = S_look(tenv, e->u.record.typ);
      if (!record_typ) {
        EM_error(e->pos, "record expression: undefined type");
        return expTy(Tr_noExp(), Ty_Record(NULL));
      }
      Ty_ty actual = actual_ty(record_typ);
      if (actual->kind != Ty_record) {
        EM_error(e->pos, "record expression: <%s> is not a record type",
                 S_name(e->u.record.typ));
        return expTy(Tr_noExp(), Ty_Record(NULL));
      }
      Ty_fieldList ty_fl = NULL;
      A_efieldList fl = NULL;
      Tr_expList tr_el = NULL;
      int n_fields = 0;
      for (fl = e->u.record.fields, ty_fl = actual->u.record; fl && ty_fl;
           fl = fl->tail, ty_fl = ty_fl->tail, n_fields++) {
        if (fl->head->name != ty_fl->head->name) {
          EM_error(e->pos, "record expression: <%s> not a valid field name",
                   S_name(fl->head->name));
          return expTy(Tr_noExp(), Ty_Record(NULL));
        }
        expty exp = transExp(level, venv, tenv, fl->head->exp);
        if (!actual_eq(exp.ty, ty_fl->head->ty)) {
          EM_error(e->pos, "record expression: both field types dismatch");
          return expTy(Tr_noExp(), Ty_Record(NULL));
        }
        tr_el = Tr_ExpList(exp.exp, tr_el);
      }
      return expTy(Tr_recordExp(tr_el, n_fields), actual);
    }
    case A_seqExp: {
      expty exp = expTy(Tr_noExp(), Ty_Void());
      A_expList el = NULL;
      Tr_expList tr_el = NULL;
      for (el = e->u.seq; el; el = el->tail) {
        exp = transExp(level, venv, tenv, el->head);
        tr_el = Tr_ExpList(exp.exp, tr_el);
      }
      if (tr_el == NULL) {
        tr_el = Tr_ExpList(exp.exp, tr_el);
      }
      return expTy(Tr_seqExp(tr_el), exp.ty);
    }
    case A_assignExp: {
      expty var = transVar(level, venv, tenv, e->u.assign.var);
      expty exp = transExp(level, venv, tenv, e->u.assign.exp);
      if (!actual_eq(var.ty, exp.ty)) {
        EM_error(
            e->pos,
            "assign expression: dismatch type between variable and expression");
        exit(1);
      }
      return expTy(Tr_assignExp(var.exp, exp.exp), Ty_Void());
    }
    case A_ifExp: {
      expty test = transExp(level, venv, tenv, e->u.iff.test);
      if (test.ty->kind != Ty_int) {
        EM_error(e->pos, "condition expression: test section must be integer");
        exit(1);
      }
      expty then = transExp(level, venv, tenv, e->u.iff.then);
      if (e->u.iff.elsee) {
        expty elsee = transExp(level, venv, tenv, e->u.iff.elsee);
        if (!actual_eq(then.ty, elsee.ty)) {
          EM_error(
              e->pos,
              "condition expression: then-else section must be the same type");
          exit(1);
        }
        return expTy(Tr_ifExp(test.exp, then.exp, elsee.exp), then.ty);
      } else {
        if (then.ty->kind != Ty_void) {
          EM_error(e->pos, "condition expression: then section must be void");
          exit(1);
        }
        return expTy(Tr_ifExp(test.exp, then.exp, NULL), Ty_Void());
      }
    }
    case A_whileExp: {
      expty test = transExp(level, venv, tenv, e->u.whilee.test);
      if (test.ty->kind != Ty_int) {
        EM_error(e->u.whilee.test->pos,
                 "while loop: test section must produce integer");
        exit(1);
      }
      inside++;  // inside loop
      Tr_exp done = Tr_doneExp();
      brk[inside] = done;  // this level of nesting
      expty body = transExp(level, venv, tenv, e->u.whilee.body);
      inside--;  // outside
      if (body.ty->kind != Ty_void) {
        EM_error(e->u.whilee.body->pos,
                 "while loop: body section must produce no value");
        exit(1);
      }
      return expTy(Tr_whileExp(test.exp, done, body.exp), Ty_Void());
    }
    case A_doWhileExp: {
      Tr_exp done = Tr_doneExp();
      brk[inside] = done;  // this level of nesting
      expty body = transExp(level, venv, tenv, e->u.whilee.body);
      inside--;  // outside
      if (body.ty->kind != Ty_void) {
        EM_error(e->u.dowhilee.body->pos,
                 "while loop: body section must produce no value");
        exit(1);
      }
      expty test = transExp(level, venv, tenv, e->u.dowhilee.test);
      if (test.ty->kind != Ty_int) {
        EM_error(e->u.dowhilee.test->pos,
                 "while loop: test section must produce integer");
        exit(1);
      }
      return expTy(Tr_doWhileExp(body.exp, test.exp, done), Ty_Void());
    }
    case A_forExp: {
      /**
       * convert for to while
       *
       * for i := lo to hi do body
       *
       * let var i := lo
       *     var limit := hi
       * in
       *     while i <= limit do (body; i := i + 1)
       * end
       *
       * tip: 你可以使用前面的 prabsyn 将上述打印出来
       * note: 使用这样的转化可以省很多事 我之前直接使用for语句很麻烦
       */
      A_dec i = A_VarDec(e->pos, e->u.forr.var, NULL, e->u.forr.lo);
      A_dec limit = A_VarDec(e->pos, S_Symbol("limit"), NULL, e->u.forr.hi);
      A_decList let_declare = A_DecList(i, A_DecList(limit, NULL));

      A_exp increment_exp = A_AssignExp(
          e->pos, A_SimpleVar(e->pos, e->u.forr.var),
          A_OpExp(e->pos, A_plusOp,
                  A_VarExp(e->pos, A_SimpleVar(e->pos, e->u.forr.var)),
                  A_IntExp(e->pos, 1)));
      A_exp while_test = A_OpExp(
          e->pos, A_leOp, A_VarExp(e->pos, A_SimpleVar(e->pos, e->u.forr.var)),
          A_VarExp(e->pos, A_SimpleVar(e->pos, S_Symbol("limit"))));
      A_exp while_body = A_SeqExp(
          e->pos, A_ExpList(e->u.forr.body, A_ExpList(increment_exp, NULL)));
      A_exp let_body = A_SeqExp(
          e->pos, A_ExpList(A_WhileExp(e->pos, while_test, while_body), NULL));

      A_exp let_exp = A_LetExp(e->pos, let_declare, let_body);
      expty exp = transExp(level, venv, tenv, let_exp);
      return exp;
    }
    case A_breakExp: {
      if (!inside) {
        EM_error(e->pos, "break expression: break expression outside loop");
        exit(1);
      }
      return expTy(Tr_breakExp(brk[inside]), Ty_Void());
    }
    case A_letExp: {
      A_decList d = NULL;
      Tr_expList head = NULL;
      S_beginScope(venv);
      S_beginScope(tenv);
      for (d = e->u.let.decs; d; d = d->tail) {
        head = Tr_ExpList(transDec(level, venv, tenv, d->head), head);
      }
      expty exp = transExp(level, venv, tenv, e->u.let.body);
      head = Tr_ExpList(exp.exp, head);
      S_endScope(tenv);
      S_endScope(venv);
      return expTy(Tr_letExp(head), exp.ty);
    }
    case A_arrayExp: {
      Ty_ty array_typ = S_look(tenv, e->u.array.typ);
      if (!array_typ) {
        EM_error(e->pos, "array expression: undefined type %s",
                 S_name(e->u.array.typ));
        exit(1);
      }
      Ty_ty actual = actual_ty(array_typ);
      if (actual->kind != Ty_array) {
        EM_error(e->pos,
                 "array expression: array type required but given another %s",
                 S_name(e->u.array.typ));
        exit(1);
      }
      expty size_typ = transExp(level, venv, tenv, e->u.array.size);
      if (size_typ.ty->kind != Ty_int) {
        EM_error(e->u.array.size->pos,
                 "array expression: integer required with array size");
        exit(1);
      }
      expty init_typ = transExp(level, venv, tenv, e->u.array.init);
      if (!actual_eq(init_typ.ty, actual->u.array)) {
        EM_error(
            e->u.array.init->pos,
            "array expression: initialize type does not match with given type");
        exit(1);
      }
      return expTy(Tr_arrayExp(size_typ.exp, init_typ.exp), actual);
    }
  }
  assert(0);  // should have returned from some clause of the switch
}

/**
 * translate abstract declaration to real declaration
 *
 * note: If applied to function and type declarations, the result will be
 * Tr_noExp() If applied to variables, the result will be assignment expression
 *
 * return: Tr_exp
 */
static Tr_exp transDec(Tr_level level, S_table venv, S_table tenv, A_dec d) {
  switch (d->kind) {
    case A_varDec: {
      Ty_ty dec_ty = NULL;  // declare type maybe NULL
      if (d->u.var.typ != NULL) {
        dec_ty = S_look(tenv, d->u.var.typ);
        if (!dec_ty) {
          EM_error(d->pos, "variable declare: undefined type %s",
                   S_name(d->u.var.typ));
          exit(1);
        }
      }
      expty init_exp = transExp(level, venv, tenv, d->u.var.init);
      // check declare type and initialize expression equal
      if (dec_ty != NULL) {
        if (!actual_eq(dec_ty, init_exp.ty)) {
          EM_error(
              d->pos,
              "variable declare: dismatch type between declare and initialze");
          exit(1);
        }
      } else {
        if (init_exp.ty->kind == Ty_nil) {
          EM_error(d->pos,
                   "variable declare: illegal nil type: nil must be assign to "
                   "a explictly record type");
          exit(1);
        }
      }
      Tr_access m_access = Tr_allocLocal(
          level, TRUE);  // to keep things easy, all vars are escaping
      S_enter(venv, d->u.var.var, E_VarEntry(m_access, init_exp.ty));
      return Tr_assignExp(Tr_simpleVar(m_access, level), init_exp.exp);
    }
    case A_typeDec: {
      A_nametyList type_list = NULL;
      bool cycle_decl = TRUE;
      int index = 0;
      void *typenames[10];  // store typenames in list, check for redeclaration

      // example: type list = { first: int, rest: list }
      // 1: add header (type list =) into environment e1
      for (type_list = d->u.type; type_list; type_list = type_list->tail) {
        S_enter(tenv, type_list->head->name,
                Ty_Name(type_list->head->name, NULL));
        for (int i = 0; i < index; i++) {
          if (typenames[i] == (void *)type_list->head->name) {
            EM_error(type_list->head->ty->pos,
                     "type declare: redeclaration type <%s>, there are two "
                     "types with the same name in the same (consecutive) batch "
                     "of mutually recursive types.",
                     S_name(type_list->head->name));
            exit(1);
          }
        }
        typenames[index++] = (void *)type_list->head->name;
      }
      // 2: translate body ({ first: int, rest: list }) to fill the place-holder
      // in the e1
      for (type_list = d->u.type; type_list; type_list = type_list->tail) {
        Ty_ty t = transTy(tenv, type_list->head->ty);
        Ty_ty name_type = S_look(tenv, type_list->head->name);
        name_type->u.name.ty = t;
        if (t->kind != Ty_name) cycle_decl = FALSE;
      }
      if (cycle_decl) {
        EM_error(d->pos,
                 "type declare: illegal cycle type declaration: must contain "
                 "at least one built-in type");
        exit(1);
      }
      return Tr_noExp();
    }
    case A_functionDec: {
      A_fundecList fun_list = NULL;
      int index = 0;
      void *typenames[10];  // store typenames in list, check for redeclaration

      // example: function treeLeaves(t: tree): int = treelistLeaves(t.children)
      // 1: add header (function treeLeaves(t: tree): int =)
      for (fun_list = d->u.function; fun_list; fun_list = fun_list->tail) {
        // i need elements of E_FunEntry, because its value environment
        A_fieldList fl = NULL;
        Ty_ty ty = NULL;

        Ty_tyList head = NULL, tail = NULL;
        Ty_ty r = NULL;

        // boolList of parameters, indicates vars escape or not
        U_boolList m_head = NULL, m_tail = NULL;

        // return type
        if (fun_list->head->result) {
          r = S_look(tenv, fun_list->head->result);
          if (!r) {
            EM_error(fun_list->head->pos,
                     "function declare: undefined return type %s",
                     S_name(fun_list->head->result));
            exit(1);
          }
        } else {
          r = Ty_Void();
        }
        // parameters
        for (fl = fun_list->head->params; fl; fl = fl->tail) {
          ty = S_look(tenv, fl->head->typ);
          if (!ty) {
            EM_error(fl->head->pos,
                     "function declare: undefined parameter type %s",
                     S_name(fl->head->typ));
            exit(1);
          }
          if (head) {
            tail->tail = Ty_TyList(ty, NULL);
            tail = tail->tail;
          } else {
            head = Ty_TyList(ty, NULL);
            tail = head;
          }
          if (m_head) {
            m_tail->tail = U_BoolList(
                TRUE, NULL);  // to keep things easy, vars are escaping
            m_tail = m_tail->tail;
          } else {
            m_head = U_BoolList(TRUE, NULL);
            m_tail = m_head;
          }
        }

        // 遇到function declaration需要在当前level上创建一个新的level
        Temp_label m_label = Temp_newlabel();
        Tr_level m_level = Tr_newLevel(level, m_label, m_head);
        S_enter(venv, fun_list->head->name,
                E_FunEntry(m_level, m_label, head, r));

        for (int i = 0; i < index; i++) {
          if (typenames[i] == (void *)fun_list->head->name) {
            EM_error(fun_list->head->pos,
                     "type declare: redeclaration type <%s>, there are two "
                     "types with the same name in the same (consecutive) batch "
                     "of mutually recursive types.",
                     S_name(fun_list->head->name));
            exit(1);
          }
        }
        typenames[index++] = (void *)fun_list->head->name;
      }
      // 2: translate body (treelistLeaves(t.children))
      for (fun_list = d->u.function; fun_list; fun_list = fun_list->tail) {
        E_enventry fun_entry = S_look(venv, fun_list->head->name);
        S_beginScope(venv);
        // add parameters into environment
        A_fieldList fl;
        Ty_tyList tl = fun_entry->u.fun.formals;
        Tr_accessList m_accessList = Tr_formals(fun_entry->u.fun.level);
        for (fl = fun_list->head->params; fl;
             fl = fl->tail, tl = tl->tail, m_accessList = m_accessList->tail) {
          S_enter(venv, fl->head->name,
                  E_VarEntry(m_accessList->head, tl->head));
        }
        // translate body
        expty exp = transExp(level, venv, tenv, fun_list->head->body);
        // compare return type and body type
        if (!actual_eq(fun_entry->u.fun.results, exp.ty)) {
          EM_error(d->pos,
                   "function declare: body type and return type with <%s>",
                   S_name(fun_list->head->name));
          exit(1);
        }
        S_endScope(venv);
      }
      return Tr_noExp();
    }
  }
}

/**
 * translate abstract type to real type
 */
static Ty_ty transTy(S_table tenv, A_ty t) {
  switch (t->kind) {
    case A_nameTy: {
      Ty_ty ty = S_look(tenv, t->u.name);
      if (!ty) {
        EM_error(t->pos, "translate name type: undefined type %s",
                 S_name(t->u.name));
        exit(1);
      }
      return ty;
    }
    case A_recordTy: {
      A_fieldList fl;
      Ty_field ty_f;
      Ty_fieldList ty_fl_head = NULL, ty_fl_tail = NULL;
      Ty_ty ty;
      for (fl = t->u.record; fl; fl = fl->tail) {
        ty = S_look(tenv, fl->head->typ);
        if (!ty) {
          EM_error(fl->head->pos, "translate record type: undefined type %s",
                   S_name(fl->head->typ));
          exit(1);
        }
        ty_f = Ty_Field(fl->head->name, ty);
        if (ty_fl_head) {
          ty_fl_tail->tail = Ty_FieldList(ty_f, NULL);
          ty_fl_tail = ty_fl_tail->tail;
        } else {
          ty_fl_head = Ty_FieldList(ty_f, NULL);
          ty_fl_tail = ty_fl_head;
        }
      }
      return Ty_Record(ty_fl_head);
    }
    case A_arrayTy: {
      Ty_ty ty = S_look(tenv, t->u.array);
      if (!ty) {
        EM_error(t->pos, "translate array type: undefined type %s",
                 S_name(t->u.array));
        exit(1);
      }
      return Ty_Array(ty);
    }
  }
  assert(0);  // should have returned from some clause of the switch
}