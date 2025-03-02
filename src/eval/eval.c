
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Tree walking evaluator
**  File: eval.c
*/

#include <builtins.h>
#include <stdlib.h>
#include <uthash.h>
#include <string.h>
#include <tokens.h>
#include <scope.h>
#include <ast.h>
#include <objects.h>
#include <eval.h>
#include <stddef.h>
#include <stdio.h>
#include <utarray.h>
#include <utstring.h>
#include <lexer.h>
#include <gc.h>
#include <core.h>
#include <stdbool.h>

extern struct object *OBJECT_NULL;

void
eval_init(struct eval *self, struct lexer *lexer, struct scope *scope) {
  self->lexer = lexer;
  self->scope = scope;

  map_builtinfn_init();
  map_builtinfn_add("len", blen);
  map_builtinfn_add("type", btype);
  map_builtinfn_add("int", bint);
  map_builtinfn_add("float", bfloat);
  map_builtinfn_add("print", bprint);
  map_builtinfn_add("println", bprintln);
  map_builtinfn_add("push", bpush);
  map_builtinfn_add("pop", bpop);
  map_builtinfn_add("slice", bslice);
  map_builtinfn_add("readint", breadint);
  map_builtinfn_add("readfloat", breadfloat);
  map_builtinfn_add("readstring", breadstring);
}

struct object*
eval(struct eval* const self, const struct program * const program) {
  struct object *result = OBJECT_NULL;

  for(size_t i = 0; i < utarray_len(program->statements); i++) {
    gc_done(result);

    const struct statement **stmt = utarray_eltptr(program->statements, i);
    result = estatement(self, *stmt);

    switch(result->kind) {
      case ORETURNVAL: {
        struct object *value = result->returnval.value;
        gc_done(result);
        return value;
      }

      case ODETAILEDERROR: {
        return result;
      }

      default:
        break;
    }
  }

  return result;
}

struct object*
serror(const char *msg) {
  struct object *serr = gc_alloc();
  serr->kind = OSIMPLEERROR;
  utstring_new(serr->simpleerror.value);
  utstring_printf(serr->simpleerror.value, "%s", msg);
  return serr;
}

struct object*
derror(struct eval* const self, const struct location* const _loc, struct simpleerror* const serr) {
  struct object *derr = gc_alloc();
  derr->kind = ODETAILEDERROR;
  derr->detailederror.value = fmterror(self->lexer, *_loc, utstring_body(serr->value));
  return derr;
}

bool
iserr(const struct object* const obj) {
  return obj != NULL && (obj->kind == ODETAILEDERROR || obj->kind == OSIMPLEERROR);
}

void
eval_deinit(void) {
  map_builtinfn_deinit();
}
