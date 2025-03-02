
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the builtin function type
**  File: builtins.c
*/

#include <builtins.h>
#include <string.h>
#include <uthash.h>
#include <objects.h>
#include <gc.h>

struct builtinfntable {
  char *key;
  struct object *fn;
  UT_hash_handle hh;
};

static struct builtinfntable *bfntable;

void
map_builtinfn_init(void) {
  bfntable = NULL;
}

void
map_builtinfn_add(const char* name, builtinfn fn) {
  struct builtinfntable *b = (struct builtinfntable*)malloc(sizeof(struct builtinfntable));
  size_t len = strlen(name);
  b->key = (char*)malloc(sizeof(char) * (len + 1));
  strcpy(b->key, name);
  b->fn = gc_alloc();
  b->fn->kind = OBUILTINFUNCTION;
  b->fn->builtinfunction.fn = fn;

  HASH_ADD_STR(bfntable, key, b);
}

bool
map_builtinfn_contains(const char* name) {
  struct builtinfntable *b;
  HASH_FIND_STR(bfntable, name, b);
  return b != NULL;
}

struct object*
map_builtinfn_get(const char* name) {
  struct builtinfntable *b;
  HASH_FIND_STR(bfntable, name, b);
  return b->fn;
}

void
map_builtinfn_deinit(void) {
  struct builtinfntable *curr, *tmp;
  HASH_ITER(hh, bfntable, curr, tmp) {
    HASH_DEL(bfntable, curr);
    free(curr->key);
    gc_done(curr->fn);
    free(curr);
  }
  bfntable = NULL;
}
