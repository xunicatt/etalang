
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Core part of the tree walking evaluator
**  File: expressions.c
*/

#include <dlfcn.h>
#include <lexer.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <tokens.h>
#include <utstring.h>
#include <scope.h>
#include <ast.h>
#include <objects.h>
#include <stddef.h>
#include <utarray.h>
#include <gc.h>
#include <core.h>
#include <builtins.h>
#include <ffi.h>

extern struct object *OBJECT_NULL;
extern struct object *OBJECT_TRUE;
extern struct object *OBJECT_FALSE;

typedef int (*random_cstr_int_func)(const char*);

static UT_icd object_icd = {sizeof(struct object*), NULL, NULL, NULL};

static struct object* eidentifierexp(struct eval*, const struct identifierexp*);
static struct object* eliteralexp(struct eval*, struct expression*);

static struct object* eprefix_not(const struct object*);
static struct object* eprefix_sub(const struct object*);
static struct object* eprefix(struct eval*, const struct prefixexp*);

static struct object* toboolobj(const bool);

static struct object* einfix_integerobj(const enum tokenkind, const struct integerobj*, const struct integerobj*);
static struct object* einfix_floatobj(const enum tokenkind, const struct floatobj*, const struct floatobj*);
static struct object* einfix_stringobj(const enum tokenkind, const struct stringobj*, const struct stringobj*);
static struct object* einfix(struct eval*, const struct infixexp*);

static struct object* eassignment_string(struct eval*, struct object*, struct expression*, struct expression*);
static struct object* eassignment_array(struct eval*, struct object*, struct expression*, struct expression*);
static struct object* eassignment_identifier(struct eval*, const struct identifierexp*, struct expression*);
static struct object* eassignment(struct eval*, const struct assignmentexp*);

static struct object* eindexexp(struct eval*, const struct indexexp*);
static struct object* eopassignment(struct eval*, const struct opassignmentexp*);
static struct object* eblockstatement(struct eval*, const struct blockstatement*);

static struct object* eifexp(struct eval*, const struct ifexp*);
static struct object* eforexp(struct eval*, const struct forexp*);
static bool isvaltruthy(const struct object*);

static struct object* efunction(struct eval*, const struct functionobj*, const struct location*, UT_array*);
static struct object* ebuiltinfunction(struct eval*, const struct builtinfn*, const struct location*, UT_array*);
static struct object* eexternalfunction(struct eval*, const struct externalfn*, const struct location*, UT_array*);
static struct object* ecallexp(struct eval*, const struct callexp*);

struct object*
eexpression(struct eval *self, struct expression *expr) {
  switch(expr->kind) {
  case NIDENTIFIEREXP: {
    return eidentifierexp(self, &expr->identifierexp);
  }

  case NNULLLIT:
    return OBJECT_NULL;

  case NINTEGERLIT:
  case NFLOATLIT:
  case NBOOLLIT:
  case NSTRINGLIT:
  case NARRAYLIT:
  case NFUNCTIONLIT:
    return eliteralexp(self, expr);

  case NPREFIXEXP:
    return eprefix(self, &expr->prefixexp);

  case NINFIXEXP:
    return einfix(self, &expr->infixexp);

  case NASSIGNMENTEXP:
    return eassignment(self, &expr->assignmentexp);

  case NINDEXEXP:
    return eindexexp(self, &expr->indexexp);

  case NOPASSIGNMENTEXP:
    return eopassignment(self, &expr->opassignmentexp);

  case NIFEXP: {
    struct scope ifscope;
    scope_init(&ifscope, self->scope);
    struct eval ifeval = {.scope = &ifscope, .lexer = self->lexer};
    struct object *res = eifexp(&ifeval, &expr->ifexp);
    scope_deinit(&ifscope);
    return res;
  }

  case NFOREXP: {
    struct scope forscope;
    scope_init(&forscope, self->scope);
    struct eval foreval = {.scope = &forscope, .lexer = self->lexer};
    struct object *res = eforexp(&foreval, &expr->forexp);
    scope_deinit(&forscope);
    return res;
  }

  case NCALLEXP: {
    return ecallexp(self, &expr->callexp);
  }

  case NEXTERNEXP: {
    struct externexp *externexp = &expr->externexp;
    if(!scope_existsany(self->scope, externexp->libname)) {
      DERROR(self, &externexp->location, "undefined identifier");
    }

    struct object *lib = scope_get(self->scope, externexp->libname);
    if(lib->kind != OEXTERNALLIBRARY) {
      DERROR(self, &externexp->location, "expected a 'library' type");
    }

    void *fn = dlsym(lib->externallibrary.lib, externexp->funcname);
    if(dlerror()) {
      DERROR(self, &expr->externexp.location, "external function not found");
    }

    struct object *res = gc_alloc();
    res->kind = OEXTERNALFUNCTION;
    steal(res->externalfunction.argumenttypes, externexp->argumenttypes);
    steal(res->externalfunction.identifier, externexp->funcname);
    res->externalfunction.returntype = externexp->returntype;
    steal(res->externalfunction.fn, fn);

    gc_borrow(res);
    scope_set(self->scope, res->externalfunction.identifier, res);
    return res;
  }

  default:
    return OBJECT_NULL;
  }
}

static struct object*
eidentifierexp(struct eval *self, const struct identifierexp *expr) {
  if(scope_existsany(self->scope, expr->value)) {
    struct object *obj = scope_get(self->scope, expr->value);
    gc_borrow(obj);
    return obj;
  }

  if(map_builtinfn_contains(expr->value)) {
    struct object *obj = map_builtinfn_get(expr->value);
    gc_borrow(obj);
    return obj;
  }

  DERROR(self, &expr->location, "undefined identifier");
}

static struct object*
eliteralexp(struct eval *self, struct expression *lit) {
  struct object *obj = gc_alloc();

  switch(lit->kind) {
    case NINTEGERLIT: {
      obj->kind = OINT;
      obj->integerobj.value = lit->integerlit.value;
      return obj;
    }

    case NFLOATLIT: {
      obj->kind = OFLOAT;
      obj->floatobj.value = lit->floatlit.value;
      return obj;
    }

    case NBOOLLIT: {
      gc_done(obj);
      if(lit->boollit.value) return OBJECT_TRUE;
      return OBJECT_FALSE;
    }

    case NSTRINGLIT: {
      obj->kind = OSTRING;
      utstring_new(obj->stringobj.value);
      utstring_printf(obj->stringobj.value, "%s", lit->stringlit.value);
      return obj;
    }

    case NARRAYLIT: {
      obj->kind = OARRAY;
      utarray_new(obj->arrayobj.elements, &object_icd);
      for(size_t i = 0; i < utarray_len(lit->arraylit.elements); i++) {
        struct expression **expr = utarray_eltptr(lit->arraylit.elements, i);
        struct object *element = eexpression(self, *expr);
        if(iserr(element)) {
          gc_done(obj);
          obj = element;
          goto cleanup;
        }
        utarray_push_back(obj->arrayobj.elements, &element);
      }
      return obj;

      cleanup:
        while(utarray_len(lit->arraylit.elements)){
          struct object **element = utarray_back(lit->arraylit.elements);
          utarray_pop_back(lit->arraylit.elements);
          gc_done(*element);
        }
        utarray_free(lit->arraylit.elements);
        return obj;
    }

    case NFUNCTIONLIT: {
      obj->kind = OFUNCTION;
      steal(obj->functionobj.parameters, lit->functionlit.parameters);
      steal(obj->functionobj.body, lit->functionlit.body);
      obj->functionobj.scope = self->scope;
      return obj;
    }

    default:
      gc_done(obj);
      return OBJECT_NULL;
  }
}

static struct object*
eprefix_not(const struct object *value) {
  if(value == OBJECT_TRUE) return OBJECT_FALSE;
  if(value ==  OBJECT_FALSE) return OBJECT_TRUE;
  if(value == OBJECT_NULL) return OBJECT_TRUE;
  return OBJECT_FALSE;
}

static struct object*
eprefix_sub(const struct object *value) {
  struct object *res = gc_alloc();
  res->kind = value->kind;

  switch(value->kind) {
    case OINT: {
      res->integerobj.value = -value->integerobj.value;
      break;
    }

    case OFLOAT: {
      res->floatobj.value = -value->floatobj.value;
      break;
    }

    default:
      res->kind = ONULL;
      gc_done(res);
      return serror("type is not supported");
  }

  return res;
}

static struct object*
eprefix(struct eval *self, const struct prefixexp *expr) {
  struct object *value = eexpression(self, expr->right);
  gc_done(value);

  switch(expr->operator) {
    case TNOT:
      return eprefix_not(value);

    case TSUB: {
      struct object *res = eprefix_sub(value);
      if(iserr(res)) {
        struct object *derr = derror(self, &expr->location, &res->simpleerror);
        gc_done(res);
        return derr;
      }
      return res;
    }

    default:
      DERROR(self, &expr->location, "unknown operator");
  }
}

static struct object*
toboolobj(const bool val) {
  if(val) return OBJECT_TRUE;
  return OBJECT_FALSE;
}

static struct object*
einfix(struct eval  *self, const struct infixexp *expr) {
  struct object *lval = eexpression(self, expr->left);
  IFERROR(self, &expr->location, lval);

  struct object *rval = eexpression(self, expr->right);
  if(iserr(rval)) {
    gc_done(lval);
    IFERROR(self, &expr->location, rval);
  }

  if(lval->kind == OINT && rval->kind == OINT) {
    struct object *res = einfix_integerobj(expr->operator, &lval->integerobj, &rval->integerobj);
    gc_done(lval);
    gc_done(rval);
    IFERROR(self, &expr->location, res);
    return res;
  }

  if(lval->kind == OFLOAT && rval->kind == OFLOAT) {
    struct object *res = einfix_floatobj(expr->operator, &lval->floatobj, &rval->floatobj);
    gc_done(lval);
    gc_done(rval);
    IFERROR(self, &expr->location, res);
    return res;
  }

  if(lval->kind == OSTRING && rval->kind == OSTRING) {
    struct object *res = einfix_stringobj(expr->operator, &lval->stringobj, &rval->stringobj);
    gc_done(lval);
    gc_done(rval);
    IFERROR(self, &expr->location, res);
    return res;
  }

  gc_done(lval);
  gc_done(rval);

  if(lval->kind != rval->kind) {
    UT_string *errmsg;
    utstring_new(errmsg);
    utstring_printf(errmsg, "type mismatch between %s and %s", objectname(lval->kind), objectname(rval->kind));
    struct object *derr;
    GETDERROR(derr, self, &expr->location, utstring_body(errmsg));
    utstring_free(errmsg);
    return derr;
  }

  if(expr->operator == TEQL) {
    return toboolobj(lval == rval);
  }

  if(expr->operator == TNEQL) {
    return toboolobj(lval != rval);
  }

  DERROR(self, &expr->location, "unknown operator");
}

static struct object*
einfix_integerobj(
  const enum tokenkind operator,
  const struct integerobj *lval,
  const struct integerobj *rval
) {
  struct object *res = gc_alloc();
  res->kind = OINT;

  switch(operator) {
    case TADD:
      res->integerobj.value = lval->value + rval->value;
      return res;

    case TSUB:
      res->integerobj.value = lval->value - rval->value;
      return res;

    case TMUL:
      res->integerobj.value = lval->value * rval->value;
      return res;

    case TDIV:
      res->integerobj.value = lval->value / rval->value;
      return res;

    case TGRT:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value > rval->value);

    case TGRE:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value >= rval->value);

    case TLES:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value < rval->value);

    case TLEE:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value <= rval->value);

    case TEQL:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value == rval->value);

    case TNEQL:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value != rval->value);

    default:
      res->kind = ONULL;
      gc_done(res);
      return serror("unknown operator");
  }
}

static struct object*
einfix_floatobj(
  const enum tokenkind operator,
  const struct floatobj *lval,
  const struct floatobj *rval
) {
  struct object *res = gc_alloc();
  res->kind = OFLOAT;

  switch(operator) {
    case TADD:
      res->floatobj.value = lval->value + rval->value;
      return res;

    case TSUB:
      res->floatobj.value = lval->value - rval->value;
      return res;

    case TMUL:
      res->floatobj.value = lval->value * rval->value;
      return res;

    case TDIV:
      res->floatobj.value = lval->value / rval->value;
      return res;

    case TGRT:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value > rval->value);

    case TGRE:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value >= rval->value);

    case TLES:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value < rval->value);

    case TLEE:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value <= rval->value);

    case TEQL:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value == rval->value);

    case TNEQL:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(lval->value != rval->value);

    default:
      res->kind = ONULL;
      gc_done(res);
      return serror("unknown operator");
  }
}

static struct object*
einfix_stringobj(
  const enum tokenkind operator,
  const struct stringobj  *lval,
  const struct stringobj  *rval
) {
  struct object *res = gc_alloc();
  res->kind = OSTRING;

  switch(operator) {
    case TADD:
      utstring_new(res->stringobj.value);
      utstring_printf(res->stringobj.value, "%s%s", utstring_body(lval->value), utstring_body(rval->value));
      return res;

    case TEQL:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(!strcmp(utstring_body(lval->value), utstring_body(rval->value)));

    case TNEQL:
      res->kind = ONULL;
      gc_done(res);
      return toboolobj(strcmp(utstring_body(lval->value), utstring_body(rval->value)));

    default:
      res->kind = ONULL;
      gc_done(res);
      return serror("unknown operator");
  }
}

static struct object*
eassignment_identifier(
  struct eval *self,
  const struct identifierexp  *ident,
  struct expression *value
) {
  if(!scope_existsany(self->scope, ident->value)) {
    DERROR(self, &ident->location, "undefined identifier");
  }

  struct object *obj = scope_get(self->scope, ident->value);
  if(obj->kind == OFUNCTION || obj->kind == OBUILTINFUNCTION) {
    DERROR(self, &ident->location, "a 'function' type variable cannot be reassigned");
  }

  struct object *res = eexpression(self, value);
  IFERROR(self, &ident->location, res);

  if(obj->kind != ONULL && obj->kind != res->kind) {
    gc_done(res);
    DERROR(self, &ident->location, "a variable cannot be assigned with a new type");
  }

  gc_borrow(res);
  return scope_update(self->scope, ident->value, res);
}

static struct object*
eassignment(struct eval *self, const struct assignmentexp *expr) {
  switch(expr->left->kind) {
    case NIDENTIFIEREXP:
      return eassignment_identifier(self, &expr->left->identifierexp, expr->right);

    case NINDEXEXP: {
      struct indexexp *indexexp = &expr->left->indexexp;
      struct object *lval = eexpression(self, indexexp->left);
      IFERROR(self, &indexexp->left->arraylit.location, lval);

      switch(lval->kind) {
        case OARRAY: {
          return eassignment_array(self, lval, indexexp->index, expr->right);
        }

        case OSTRING: {
          return eassignment_string(self, lval, indexexp->index, expr->right);
        }

        default:
          gc_done(lval);
          DERROR(self, &indexexp->left->arraylit.location, "type cannot be indexed");
      }
    }

    default:
      DERROR(self, &expr->left->arraylit.location, "lvalue cannot be assigned with a value");
  }
}

static struct object*
eassignment_string(
  struct eval *self,
  struct object *string,
  struct expression *index,
  struct expression *right
) {
  struct object *idxval = eexpression(self, index);
  if(iserr(idxval)) {
    IFERROR(self, &index->arraylit.location, idxval);
  }

  if(idxval->kind != OINT) {
    gc_done(idxval);
    DERROR(self, &index->arraylit.location, "expected an int type");
  }

  int64_t len = utstring_len(string->stringobj.value), idx = idxval->integerobj.value;
  gc_done(idxval);

  if(idx < 0 || idx >= len) {
    DERROR(self, &index->arraylit.location, "index out of range");
  }

  struct object *val = eexpression(self, right);
  if(iserr(val)) {
    return val;
  }

  if(val->kind != OSTRING) {
    gc_done(val);
    DERROR(self, &right->arraylit.location, "expected a string type");
  }

  utstring_body(string->stringobj.value)[idx] = utstring_body(val->stringobj.value)[0];
  gc_done(val);
  return string;
}

static struct object*
eassignment_array(
  struct eval *self,
  struct object *array,
  struct expression *index,
  struct expression *right
) {
  struct object *idxval = eexpression(self, index);
  if(iserr(idxval)) {
    IFERROR(self, &index->arraylit.location, idxval);
  }

  if(idxval->kind != OINT) {
    gc_done(idxval);
    DERROR(self, &index->arraylit.location, "expected an int type");
  }

  int64_t len = utarray_len(array->arrayobj.elements), idx = idxval->integerobj.value;
  gc_done(idxval);

  if(idx < 0 || idx >= len) {
    DERROR(self, &index->arraylit.location, "index out of range");
  }

  struct object *val = eexpression(self, right);
  if(iserr(val)) {
    return val;
  }

  struct object **obj = utarray_eltptr(array->arrayobj.elements, idx);
  gc_done(*obj);
  utarray_replace(array->arrayobj.elements, idx, &val);
  return array;
}

static struct object*
eindexexp(struct eval *self, const struct indexexp *expr) {
  struct object *obj = eexpression(self, expr->left);
  IFERROR(self, &expr->left->arraylit.location, obj);

  struct object *idxval = eexpression(self, expr->index);
  if(iserr(idxval)) {
    gc_done(obj);
    IFERROR(self, &expr->index->arraylit.location, idxval);
  }

  if(idxval->kind != OINT) {
    gc_done(obj);
    gc_done(idxval);
    DERROR(self, &expr->index->arraylit.location, "expected an int type");
  }

  switch(obj->kind) {
    case OSTRING: {
      int64_t len = utstring_len(obj->stringobj.value), idx = idxval->integerobj.value;

      if(idx < 0 || idx >= len) {
        gc_done(idxval);
        gc_done(obj);
        DERROR(self, &expr->index->arraylit.location, "index out of range");
      }

      char c = utstring_body(obj->stringobj.value)[idx];
      struct object *res = gc_alloc();
      res->kind = OSTRING;
      utstring_new(res->stringobj.value);
      utstring_printf(res->stringobj.value, "%c", c);
      gc_done(idxval);
      gc_done(obj);
      return res;
    }

    case OARRAY: {
      int64_t len = utarray_len(obj->arrayobj.elements), idx = idxval->integerobj.value;

      if(idx < 0 || idx >= len) {
        gc_done(idxval);
        gc_done(obj);
        DERROR(self, &expr->index->arraylit.location, "index out of range");
      }

      struct object **element = utarray_eltptr(obj->arrayobj.elements, idx);
      gc_done(idxval);
      gc_done(obj);
      gc_borrow(*element);
      return *element;
    }

    default:
      gc_done(idxval);
      gc_done(obj);
      DERROR(self, &expr->left->arraylit.location, "type cannot be indexed");
  }
}

static struct object*
eopassignment(struct eval *self, const struct opassignmentexp *expr) {
  if(expr->left->kind != NIDENTIFIEREXP) {
    DERROR(self, &expr->left->arraylit.location, "expected an variable");
  }

  struct infixexp infinixexp = {
    .kind = NINFIXEXP,
    .left = expr->left,
    .right = expr->right,
    .operator = expr->operator,
    .location = expr->location
  };
  struct object *res = einfix(self, &infinixexp);
  if(iserr(res)) {
    return res;
  }

  struct object *oldval = scope_get(self->scope, expr->left->identifierexp.value);
  if(oldval->kind != res->kind) {
    gc_done(res);
    DERROR(self, &expr->location, "type mismatch");
  }

  gc_borrow(res);
  return scope_update(self->scope, expr->left->identifierexp.value, res);
}

static struct object*
eblockstatement(struct eval *self, const struct blockstatement *block) {
  struct object *result = OBJECT_NULL;

  for(size_t i = 0; i < utarray_len(block->statements); i++) {
    gc_done(result);
    struct statement **stmt = utarray_eltptr(block->statements, i);
    result = estatement(self, *stmt);

    if(result->kind == ORETURNVAL || result->kind == ODETAILEDERROR) {
      return result;
    }
  }

  return result;
}

static bool isvaltruthy(const struct object *obj) {
  if(obj == OBJECT_NULL) return false;
  if(obj == OBJECT_TRUE) return true;
  if(obj == OBJECT_FALSE) return false;
  return true;
}

static struct object*
eifexp(struct eval *self, const struct ifexp *expr) {
  struct object *condval = eexpression(self, expr->condition);
  IFERROR(self, &expr->condition->arraylit.location, condval);

  if(isvaltruthy(condval)) {
    gc_done(condval);
    return eblockstatement(self, expr->consequence);
  }

  gc_done(condval);
  if(expr->alternative) {
    return eblockstatement(self, expr->alternative);
  }

  return OBJECT_NULL;
}

static struct object*
eforexp(struct eval *self, const struct forexp *expr) {
  if(expr->initialization) {
    struct object *init = estatement(self, expr->initialization);
    if(iserr(init)) {
      return init;
    }
    gc_done(init);
  }

  struct object *res = OBJECT_NULL;
  while(true) {
    if(expr->condition) {
      struct object *cond = eexpression(self, expr->condition);
      if(iserr(cond)) {
        return cond;
      }

      gc_done(cond);
      if(!isvaltruthy(cond)) {
        return res;
      }
    }

    struct scope innerscope;
    scope_init(&innerscope, self->scope);
    struct eval innereval = {.scope = &innerscope, .lexer = self->lexer};
    res = eblockstatement(&innereval, expr->body);
    if(iserr(res)) {
      return res;
    }

    if(res->kind == ORETURNVAL) {
      scope_deinit(&innerscope);
      return res;
    }

    scope_deinit(&innerscope);

    if(expr->updation) {
      struct object *updt = estatement(self, expr->updation);
      if(iserr(updt)) {
        return updt;
      }

      gc_done(updt);
    }
  }
}

static struct object*
efunction(struct eval *self, const struct functionobj *func, const struct location *loc, UT_array *args) {
  if(utarray_len(func->parameters) != utarray_len(args)) {
    UT_string *msg;
    utstring_new(msg);
    utstring_printf(
      msg,
      "expected %d arguments but got %d",
      utarray_len(func->parameters),
      utarray_len(args)
    );
    struct object *derr;
    GETDERROR(derr, self, loc, utstring_body(msg));
    utstring_free(msg);
    return derr;
  }

  UT_array *argobjs;
  struct object *ret = OBJECT_NULL;
  utarray_new(argobjs, &object_icd);

  for(size_t i = 0; i < utarray_len(args); i++) {
    struct expression **argument = utarray_eltptr(args, i);
    struct object *arg = eexpression(self, *argument);
    if(iserr(arg)) {
      ret = arg;
      goto cleanup;
    }
    utarray_push_back(argobjs, &arg);
  }

  struct scope fnscope;
  scope_init(&fnscope, func->scope);
  for(size_t i = 0; i < utarray_len(func->parameters); i++) {
    struct identifierexp **ident = utarray_eltptr(func->parameters, i);
    struct object **arg = utarray_eltptr(argobjs, i);
    gc_borrow(*arg);
    scope_set(&fnscope, (*ident)->value, *arg);
  }

  struct eval fneval = {.scope = &fnscope, .lexer = self->lexer};
  ret = eblockstatement(&fneval, func->body);
  scope_deinit(&fnscope);

cleanup:
  while(utarray_len(argobjs)) {
    struct object **arg = utarray_back(argobjs);
    utarray_pop_back(argobjs);
    gc_done(*arg);
  }
  utarray_free(argobjs);
  return ret;
}

static struct object*
ebuiltinfunction(struct eval *self, const struct builtinfn *func, const struct location *loc, UT_array *args) {
  UT_array *argobjs;
  struct object *ret = OBJECT_NULL;

  utarray_new(argobjs, &object_icd);
  for(size_t i = 0; i < utarray_len(args); i++) {
    struct expression **argument = utarray_eltptr(args, i);
    struct object *arg = eexpression(self, *argument);
    if(iserr(arg)) {
      ret = arg;
      goto cleanup;
    }
    utarray_push_back(argobjs, &arg);
  }

  ret = func->fn(argobjs);
  if(iserr(ret) && ret->kind == OSIMPLEERROR) {
    struct object *derr = derror(self, loc, &ret->simpleerror);
    gc_done(ret);
    ret = derr;
  }

cleanup:
  while(utarray_len(argobjs)) {
    struct object **arg = utarray_back(argobjs);
    utarray_pop_back(argobjs);
    gc_done(*arg);
  }
  utarray_free(argobjs);

  return ret;
}

static enum objectkind abi_get_objectkind(enum tokenkind a) {
  if(a == TTINT) return OINT;
  if(a == TTFLOAT) return OFLOAT;
  if(a == TTBOOL) return OBOOL;
  if(a == TTSTRING) return OSTRING;
  return ONULL;
}

static void*
abi_get_value(struct object *o) {
  switch(o->kind) {
    case OINT:
      return &o->integerobj.value;

    case OFLOAT:
      return &o->floatobj.value;

    case OSTRING:
      return &o->stringobj.value->d;

    case OBOOL:
      return &o->boolobj.value;

    default:
      return NULL;
  }
}

static ffi_type*
abi_get_ffitype(const struct object *o) {
  switch(o->kind) {
    case OINT:
      return &ffi_type_sint;

    case OFLOAT:
      return &ffi_type_double;

    case OSTRING:
      return &ffi_type_pointer;

    case OBOOL:
      return &ffi_type_sint;

    default:
      return &ffi_type_void;
  }
}

static struct object*
eexternalfunction(struct eval *self, const struct externalfn *func, const struct location *loc, UT_array *args) {
  size_t len = utarray_len(args);
  struct object *argsobj[len];
  ffi_type *argstype[len];
  void *argsval[len];
  struct object *ret = OBJECT_NULL;

  for(size_t i = 0; i < utarray_len(args); i++) {
    struct expression **argument = utarray_eltptr(args, i);
    enum tokenkind type = *(enum tokenkind*)utarray_eltptr(func->argumenttypes, i);
    struct object *arg = eexpression(self, *argument);
    if(iserr(arg)) {
      ret = arg;
      goto cleanup;
    }

    if(abi_get_objectkind(type) != arg->kind) {
      GETDERROR(ret, self, loc, "arguments types are not same");
      gc_done(arg);
      goto cleanup;
    }

    argsobj[i] = arg;
    argstype[i] = abi_get_ffitype(arg);
    argsval[i] = abi_get_value(arg);
  }

  ffi_cif cif;
  ffi_type *rettype = abi_get_ffitype(&(struct object){.kind = abi_get_objectkind(func->returntype)});
  ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, len, rettype, argstype);
  if(status != FFI_OK) {
    GETDERROR(ret, self, loc, "failed to ffi_prep_cif\n");
    goto cleanup;
  }

  ret = gc_alloc();
  ret->kind = abi_get_objectkind(func->returntype);
  void *x = abi_get_value(ret);

  ffi_call(&cif, FFI_FN(func->fn), x, argsval);

cleanup:
  for(size_t i = 0; i < len; i++) {
    gc_done(argsobj[i]);
  }

  return ret;
}

static struct object*
ecallexp(struct eval *self, const struct callexp *expr) {
  struct object *func = eexpression(self, expr->function);
  if(iserr(func)) {
    return func;
  }

  struct object *res = OBJECT_NULL;
  switch(func->kind) {
    case OFUNCTION: {
      res = efunction(self, &func->functionobj, &expr->function->arraylit.location, expr->arguments);
      break;
    }

    case OBUILTINFUNCTION: {
      res = ebuiltinfunction(self, &func->builtinfunction, &expr->function->arraylit.location, expr->arguments);
      break;
    }

    case OEXTERNALFUNCTION: {
      res = eexternalfunction(self, &func->externalfunction, &expr->function->arraylit.location, expr->arguments);
      break;
    }

    default:
      gc_done(func);
      DERROR(self, &expr->function->arraylit.location, "not a function");
  }

  gc_done(func);
  if(res->kind == ORETURNVAL) {
    gc_done(res);
    res = res->returnval.value;
  }
  return res;
}
