
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the token to prefix parsing functions type
**  File: prefixfns.c
*/

#include <prefixfns.h>
#include <uthash.h>
#include <tokens.h>

struct prefixfn {
  enum tokenkind key;
  prefixfn fn;
  UT_hash_handle hh;
};

static struct prefixfn *prefixfns;

void
map_prefixfns_init(void) {
  prefixfns = NULL;
}

void
map_prefixfns_add(enum tokenkind key, prefixfn fn) {
  struct prefixfn *i = (struct prefixfn*)malloc(sizeof(struct prefixfn));
  i->key = key;
  i->fn = fn;

  HASH_ADD_INT(prefixfns, key, i);
}

bool
map_prefixfns_contains(enum tokenkind key) {
  struct prefixfn *i;
  HASH_FIND_INT(prefixfns, &key, i);
  return i != NULL;
}

prefixfn
map_prefixfns_get(enum tokenkind key) {
  struct prefixfn *i;
  HASH_FIND_INT(prefixfns, &key, i);
  return i->fn;
}

void
map_prefixfns_deinit(void) {
  struct prefixfn *curr, *tmp;
  HASH_ITER(hh, prefixfns, curr, tmp) {
    HASH_DEL(prefixfns, curr);
    free(curr);
  }
}
