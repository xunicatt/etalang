
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for creating different primitive objects in eta
**  File: objects.c
*/

#include <inttypes.h>
#include <stdio.h>
#include <utarray.h>
#include <stddef.h>
#include <utstring.h>
#include <objects.h>
#include <ast.h>

extern void gc_done(struct object *);

const char* const objectstr[__OBJECTTYPECOUNT__] = {
  [ONULL] = "null",
  [OINT] = "int",
  [OFLOAT] = "float",
  [OBOOL] = "bool",
  [OSTRING] = "string",
  [OARRAY] = "array",
  [ORETURNVAL] = "return",
  [OSIMPLEERROR] = "error",
  [ODETAILEDERROR] = "error",
  [OFUNCTION] = "function",
  [OBUILTINFUNCTION] = "builtin function",
  [OEXTERNALFUNCTION] = "external function",
};

const char*
objectname(enum objectkind o) {
  if(o >= __OBJECTTYPECOUNT__) {
    return objectname(ONULL);
  }
  return objectstr[o];
}

UT_string*
object_tostr(const struct object * const obj) {
  UT_string *res;
  utstring_new(res);

  switch(obj->kind) {
    case ONULL:
      utstring_printf(res, "null");
      return  res;

    case OINT:
      utstring_printf(res, "%" PRId64, obj->integerobj.value);
      return res;

    case OFLOAT:
      utstring_printf(res, "%f", obj->floatobj.value);
      return res;

    case OBOOL:
      utstring_printf(res, "%s", obj->boolobj.value ? "true" : "false");
      return res;

    case OSTRING:
      utstring_printf(res, "%s", utstring_body(obj->stringobj.value));
      return res;

    case OARRAY: {
      utstring_printf(res, "[");
      if(obj->arrayobj.elements) {
        for(size_t i = 0; i < utarray_len(obj->arrayobj.elements); i++) {
          const struct object **eleobj = utarray_eltptr(obj->arrayobj.elements, i);
          UT_string *elestr = object_tostr(*eleobj);
          utstring_printf(
            res,
            "%s%s%s%s",
            (*eleobj)->kind == OSTRING ? "\"" : "",
            utstring_body(elestr),
            (*eleobj)->kind == OSTRING ? "\"" : "",
            i < utarray_len(obj->arrayobj.elements) - 1 ? ", " : ""
          );
          utstring_free(elestr);
        }
      }
      utstring_printf(res, "]");
      return res;
    }

    case ORETURNVAL: {
      if(obj->returnval.value) {
        utstring_free(res);
        return object_tostr(obj->returnval.value);
      }

      utstring_printf(res, "null");
      return res;
    }

    case OSIMPLEERROR: {
      utstring_printf(res, "%s", utstring_body(obj->simpleerror.value));
      return res;
    }

    case ODETAILEDERROR: {
      utstring_printf(res, "%s", utstring_body(obj->detailederror.value));
      return res;
    }

    case OFUNCTION:
    case OEXTERNALFUNCTION:
    case OBUILTINFUNCTION: {
      utstring_printf(res, "function");
      return res;
    }

    default: {
      utstring_printf(res, "invalid object");
      return res;
    }
  }
}

void
object_destroy(struct object* const self) {
  switch(self->kind) {
    case OSTRING: {
      utstring_free(self->stringobj.value);
      return;
    }

    case OARRAY: {
      for(size_t i = 0; i < utarray_len(self->arrayobj.elements); i++) {
        struct object **obj = utarray_eltptr(self->arrayobj.elements, i);
        gc_done(*obj);
      }
      utarray_free(self->arrayobj.elements);
      return;
    }

    case OSIMPLEERROR: {
      utstring_free(self->simpleerror.value);
      return;
    }

    case ODETAILEDERROR: {
      utstring_free(self->detailederror.value);
      return;
    }

    case OFUNCTION: {
      while(utarray_len(self->functionobj.parameters)) {
        struct identifierexp **ident = utarray_back(self->functionobj.parameters);
        utarray_pop_back(self->functionobj.parameters);
        free((*ident)->value);
        free(*ident);
      }
      utarray_free(self->functionobj.parameters);
      blockstatement_free(self->functionobj.body);
      free(self->functionobj.body);
      return;
    }

    case OEXTERNALFUNCTION: {
      utarray_free(self->externalfunction.argumenttypes);
      free(self->externalfunction.identifier);
      return;
    }

    default:
      return;
  }
}
