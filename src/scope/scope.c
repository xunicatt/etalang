
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: a special and important module to store named objects
**                i.e, variables
**  File: scope.c
*/

#include <objects.h>
#include <scope.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uthash.h>

extern struct object *OBJECT_NULL;
extern void gc_done(struct object *);

struct namedobject {
  char *key;
  struct object *value;
  UT_hash_handle hh;
};

void
scope_init(struct scope* const self, struct scope* const outer) {
  self->outer = outer;
  self->table = NULL;
}

bool
scope_exists(const struct scope* const self, const char *name) {
  struct namedobject *obj;
  HASH_FIND_STR(self->table, name, obj);
  return obj != NULL;
}

bool
scope_existsany(const struct scope* const self,const char *name) {
  if(scope_exists(self, name)) {
    return true;
  }

  if(self->outer) {
    return scope_existsany(self->outer, name);
  }

  return false;
}

struct object*
scope_get(const struct scope* const self, const char *name) {
  struct namedobject *obj;

  if(scope_exists(self, name)) {
    HASH_FIND_STR(self->table, name, obj);
    return obj->value;
  }

  if(self->outer) {
    return scope_get(self->outer, name);
  }

  return OBJECT_NULL;
}

struct object*
scope_set(struct scope* const self, const char *name, struct object *object) {
  struct namedobject *nobj = (struct namedobject*)malloc(sizeof(struct namedobject));
  size_t len = strlen(name); //[TODO]: PASS THE LEN AS ARG LATTER
  nobj->key = (char*)malloc(sizeof(char) * (len + 1));
  strcpy(nobj->key, name);
  nobj->value = object;
  HASH_ADD_STR(self->table, key, nobj);
  return object;
}

struct object*
scope_update(struct scope* const self, const char *name, struct object *object) {
  struct namedobject *nobj;

  HASH_FIND_STR(self->table, name, nobj);
  if(nobj != NULL) {
    gc_done(nobj->value);
    nobj->value = object;
    return object;
  }

  if(self->outer) {
    return scope_update(self->outer, name, object);
  }

  return OBJECT_NULL;
}

void
scope_deinit(struct scope* const self) {
  struct namedobject *curr, *tmp;
  HASH_ITER(hh, self->table, curr, tmp) {
    HASH_DEL(self->table, curr);
    gc_done(curr->value);
    free(curr->key);
    free(curr);
  }
}
