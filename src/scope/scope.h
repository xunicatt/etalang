
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: a special and important module to store named objects
**                i.e, variables
**  File: scope.h
*/

#ifndef __SCOPE_H__
#define __SCOPE_H__

#include <objects.h>
#include <uthash.h>
#include <stdbool.h>

struct namedobject;

struct scope {
  struct namedobject *table;
  struct scope *outer;
};

void scope_init(struct scope* const self,struct scope* const outer);
bool scope_exists(const struct scope* const self, const char *name);
bool scope_existsany(const struct scope* const self,const char *name);
struct object* scope_get(const struct scope* const self, const char *name);
struct object* scope_set(struct scope* const self, const char *name, struct object *object);
struct object* scope_update(struct scope* const self, const char *name, struct object *object);
void scope_deinit(struct scope* const self);

#endif
