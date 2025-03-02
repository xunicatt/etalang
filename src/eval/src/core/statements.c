
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Core part of the tree walking evaluator
**  File: statements.c
*/

#include <scope.h>
#include <ast.h>
#include <objects.h>
#include <stddef.h>
#include <utarray.h>
#include <gc.h>
#include <core.h>
#include <builtins.h>

extern struct object *OBJECT_NULL;

static struct object* eletstatement(struct eval*, const struct letstatement*);

struct object*
estatement(struct eval *self, const struct statement *stmt) {
  switch(stmt->kind) {
    case NLETSTATEMENT:
      return eletstatement(self, &stmt->letstatement);

    case NEXPRESSIONSTATEMENT:
      return eexpression(self, stmt->expressionstatement.expression);

    case NRETURNSTATEMENT: {
      struct object *returnval = gc_alloc();
      returnval->kind = ORETURNVAL;

      if(stmt->returnstatement.value) {
        struct object *result = eexpression(self, stmt->returnstatement.value);
        if(iserr(result)) {
          gc_done(returnval);
          return result;
        }

        returnval->returnval.value = result;
      }else{
        returnval->returnval.value = OBJECT_NULL;
      }

      return returnval;
    }

    default:
      return OBJECT_NULL;
  }
}

static struct object*
eletstatement(struct eval *self, const struct letstatement *stmt) {
  if(scope_exists(self->scope, stmt->name->value)) {
    DERROR(self, &stmt->name->location, "redeclaration of same varibale");
  }

  if(map_builtinfn_contains(stmt->name->value)) {
    DERROR(self, &stmt->name->location, "a function already exists with same name");
  }

  struct object *result = eexpression(self, stmt->value);
  if(iserr(result)) {
    return result;
  }

  gc_borrow(result);
  return scope_set(self->scope, stmt->name->value, result);
}
