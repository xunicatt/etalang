
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for creating different primitive objects in eta
**  File: objects.h
*/

#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <tokens.h>
#include <stdint.h>
#include <utstring.h>
#include <utarray.h>
#include <stdbool.h>

enum objectkind {
  ONULL = 0,
  OINT,
  OFLOAT,
  OBOOL,
  OSTRING,
  OARRAY,
  ORETURNVAL,
  OSIMPLEERROR,
  ODETAILEDERROR,
  OFUNCTION,
  OBUILTINFUNCTION,
  OEXTERNALFUNCTION,
  OEXTERNALLIBRARY,
  __OBJECTTYPECOUNT__,
};

struct object;
struct scope;

struct integerobj {
  int64_t value;
};

struct floatobj {
  double value;
};

struct boolobj {
  bool value;
};

struct stringobj {
  UT_string *value;
};

struct arrayobj {
  UT_array *elements; //object_icd
};

struct nullobj {};

struct returnval {
  struct object *value;
};

struct simpleerror {
  UT_string *value;
};

struct detailederror {
  UT_string *value;
};

struct functionobj {
  UT_array *parameters; //identifier_icd
  struct blockstatement *body;
  struct scope *scope;
};

//object_icd
typedef struct object*(*builtinfn)(const UT_array*);

struct builtinfn {
  builtinfn fn;
};

struct externalfn {
  void *fn;
  char *identifier;
  UT_array *argumenttypes;
  enum tokenkind returntype;
};

struct externallib {
  void *lib;
};

struct object {
  enum objectkind kind;
  union {
    struct integerobj integerobj;
    struct floatobj floatobj;
    struct boolobj boolobj;
    struct stringobj stringobj;
    struct arrayobj arrayobj;
    struct nullobj nullobj;
    struct returnval returnval;
    struct simpleerror simpleerror;
    struct detailederror detailederror;
    struct functionobj functionobj;
    struct builtinfn builtinfunction;
    struct externalfn externalfunction;
    struct externallib externallibrary;
  };
};

const char* objectname(enum objectkind);
UT_string* object_tostr(const struct object * const);
void object_destroy(struct object* const);

#endif
