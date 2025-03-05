
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: implementation core builtin functions for eta
**  File: builtinfn.c
*/

#include <utstring.h>
#include <core.h>
#include <objects.h>
#include <gc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <utarray.h>
#include <inttypes.h>

extern struct object *OBJECT_NULL;
static UT_array* utarray_copy(const UT_array*, size_t, size_t);
UT_icd object_icd = {sizeof(struct object*), NULL, NULL, NULL};

struct object*
blen(const UT_array *args) {
  if(utarray_len(args) != 1) {
    return serror("len(...) only accepts 1 argument");
  }

  struct object *obj = *(struct object**)utarray_front(args);
  struct object *res = gc_alloc();

  switch(obj->kind) {
    case OSTRING: {
      res->kind = OINT;
      res->integerobj.value = utstring_len(obj->stringobj.value);
      return res;
    }

    case OARRAY: {
      res->kind = OINT;
      res->integerobj.value = utarray_len(obj->arrayobj.elements);
      return res;
    }

    default:
      gc_done(res);
      return serror("type is not supported");
  }
}

struct object*
btypeof(const UT_array *args) {
  if(utarray_len(args) != 1) {
    return serror("typeof(...) only accepts 1 argument");
  }

  struct object **obj = utarray_front(args);
  struct object *res = gc_alloc();
  res->kind = OSTRING;
  utstring_new(res->stringobj.value);
  utstring_printf(res->stringobj.value, "%s", objectname((*obj)->kind));
  return res;
}

struct object*
btoint(const UT_array *args) {
  if(utarray_len(args) != 1) {
    return serror("toint(...) only accepts 1 argument");
  }

  struct object *obj = *(struct object **)utarray_front(args);
  struct object *res = gc_alloc();

  switch(obj->kind) {
    case OINT: {
      gc_done(res);
      return obj;
    }

    case OFLOAT: {
      res->kind = OINT;
      res->integerobj.value = obj->floatobj.value;
      return res;
    }

    case OBOOL: {
      res->kind = OINT;
      res->integerobj.value = obj->boolobj.value;
      return res;
    }

    default:
      return serror("type is not supported");
  }
}

struct object*
btofloat(const UT_array *args) {
  if(utarray_len(args) != 1) {
    return serror("tofloat(...) only accepts 1 argument");
  }

  struct object *obj = *(struct object **)utarray_front(args);
  struct object *res = gc_alloc();

  switch(obj->kind) {
    case OFLOAT: {
      gc_done(res);
      return obj;
    }

    case OINT: {
      res->kind = OFLOAT;
      res->floatobj.value = obj->integerobj.value;
      return res;
    }

    default:
      return serror("type is not supported");
  }
}

struct object*
bprint(const UT_array *args) {
  struct object **arg = NULL;
  while((arg = utarray_next(args, arg))) {
    UT_string *argstr = object_tostr(*arg);
    printf("%s", utstring_body(argstr));
    utstring_free(argstr);
  }
  return OBJECT_NULL;
}

struct object*
bprintln(const UT_array *args) {
  bprint(args);
  printf("\n");
  return OBJECT_NULL;
}

struct object*
bpush(const UT_array *args) {
  if(utarray_len(args) != 2) {
    return serror("push(...) only accepts 2 argument");
  }

  struct object *array = *(struct object**)utarray_eltptr(args, 0);
  if(array->kind != OARRAY) {
    return serror("expected 'array' type");
  }

  struct object *item = *(struct object**)utarray_eltptr(args, 1);
  utarray_push_back(array->arrayobj.elements, &item);
  gc_borrow(item);
  gc_borrow(array);
  return array;
}

struct object*
bpop(const UT_array *args) {
  if(utarray_len(args) != 1) {
    return serror("pop(...) only accepts 1 argument");
  }

  struct object *array = *(struct object**)utarray_front(args);
  if(array->kind != OARRAY) {
    return serror("expected 'array' type");
  }

  if(utarray_len(array->arrayobj.elements)) {
    struct object *item = *(struct object**)utarray_back(array->arrayobj.elements);
    utarray_pop_back(array->arrayobj.elements);
    gc_done(item);
  }

  gc_borrow(array);
  return array;
}

struct object*
bslice(const UT_array *args) {
  if(utarray_len(args) >= 1) {
    struct object *array = *(struct object**)utarray_front(args);
    if(array->kind != OARRAY) {
      return serror("expected 'array' type as first argument");
    }

    switch(utarray_len(args)) {
      case 1: {
        struct object *res = gc_alloc();
        res->kind = OARRAY;
        res->arrayobj.elements = utarray_copy(array->arrayobj.elements, 0, utarray_len(array->arrayobj.elements));
        return res;
      }

      case 3: {
        struct object *sidx = *(struct object**)utarray_eltptr(args, 1);
        if(sidx->kind != OINT) {
          return serror("expected 'int' type as second argument");
        }

        struct object *eidx = *(struct object**)utarray_eltptr(args, 2);
        if(eidx->kind != OINT) {
          return serror("expected 'int' type as third argument");
        }

        int64_t start = sidx->integerobj.value;
        int64_t end = eidx->integerobj.value;
        int64_t len = utarray_len(array->arrayobj.elements);

        if(start < 0 || end >= len || start >= end) {
          return serror("index out of range or invalid");
        }

        struct object *res = gc_alloc();
        res->kind = OARRAY;
        res->arrayobj.elements = utarray_copy(array->arrayobj.elements, start, end);
        return res;
      }

      default:
    }
  }

  return serror("slice(...) expects 1 or 3 arguments");
}

static UT_array*
utarray_copy(const UT_array* array, size_t start, size_t end) {
  UT_array *narray;
  utarray_new(narray, &object_icd);

  for(size_t i = start;  i < end; i++) {
    struct object **obj = utarray_eltptr(array, i);

    if((*obj)->kind == OARRAY) {
      UT_array *sarray = utarray_copy((*obj)->arrayobj.elements, 0, utarray_len((*obj)->arrayobj.elements));
      struct object *nsarray = gc_alloc();
      nsarray->kind = OARRAY;
      nsarray->arrayobj.elements = sarray;
      utarray_push_back(narray, &nsarray);
      continue;
    }

    gc_borrow(*obj);
    utarray_push_back(narray, obj);
  }

  return narray;
}

struct object*
breadint(const UT_array* args) {
  if(utarray_len(args) != 0) {
    return serror("readint(...) accepts no argument");
  }

  struct object *res = gc_alloc();
  res->kind = OINT;
  scanf("%" PRId64, &res->integerobj.value);
  return res;
}

struct object*
breadfloat(const UT_array* args) {
  if(utarray_len(args) != 0) {
    return serror("readfloat(...) accepts no argument");
  }

  struct object *res = gc_alloc();
  res->kind = OFLOAT;
  scanf("%lf", &res->floatobj.value);
  return res;
}

struct object*
breadstring(const UT_array* args) {
  if(utarray_len(args) != 0) {
    return serror("readstring(...) accepts no argument");
  }

  char c;
  struct object *res = gc_alloc();
  res->kind = OSTRING;
  utstring_new(res->stringobj.value);
  while((c = getchar()) != '\n') {
    utstring_printf(res->stringobj.value, "%c", c);
  }
  return res;
}
