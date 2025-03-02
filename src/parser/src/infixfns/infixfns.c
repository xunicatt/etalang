
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the token to infix parsing functions type
**  File: infixfns.c
*/

#include <tokens.h>
#include <uthash.h>
#include <infixfns.h>
#include <stdlib.h>

struct infixfn {
  enum tokenkind key;
  infixfn fn;
  UT_hash_handle hh;
};

static struct infixfn *infixfns;

void
map_infixfns_init(void) {
 infixfns = NULL;
}

void
map_infixfns_add(enum tokenkind key, infixfn fn) {
  struct infixfn *i = (struct infixfn*)malloc(sizeof(struct infixfn));
  i->key = key;
  i->fn = fn;

  HASH_ADD_INT(infixfns, key, i);
}

bool
map_infixfns_contains(enum tokenkind key) {
  struct infixfn *i;
  HASH_FIND_INT(infixfns, &key, i);
  return i != NULL;
}

infixfn
map_infixfns_get(enum tokenkind key) {
  struct infixfn *i;
  HASH_FIND_INT(infixfns, &key, i);
  return i->fn;
}

void
map_infixfns_deinit(void) {
  struct infixfn *curr, *tmp;
  HASH_ITER(hh, infixfns, curr, tmp) {
    HASH_DEL(infixfns, curr);
    free(curr);
  }
}
