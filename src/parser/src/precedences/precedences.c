
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the token to precedence type
**  File: precedences.c
*/

#include <precedences.h>
#include <stdlib.h>
#include <tokens.h>
#include <uthash.h>
#include <parser_def.h>

struct _precedence {
  enum tokenkind key;
  enum precedence value;
  UT_hash_handle hh;
};

static struct _precedence *precedences;

static void add(enum tokenkind key, enum precedence value);

void
map_precedences_init(void) {
  add(TASS, PASSIGNMENT);
  add(TADAS, PASSIGNMENT);
  add(TSBAS, PASSIGNMENT);
  add(TMLAS, PASSIGNMENT);
  add(TDVAS, PASSIGNMENT);
  add(TEQL, PEQUALS);
  add(TNEQL, PEQUALS);
  add(TLES, PLESSGREATER);
  add(TLEE, PLESSGREATER);
  add(TGRT, PLESSGREATER);
  add(TGRE, PLESSGREATER);
  add(TADD, PSUM);
  add(TSUB, PSUM);
  add(TDIV, PPRODUCT);
  add(TMUL, PPRODUCT);
  add(TOPAREN, PCALL);
  add(TOSQR, PINDEX);
}

static void
add(enum tokenkind key, enum precedence value) {
  struct _precedence *p = (struct _precedence*)malloc(sizeof(struct _precedence));
  p->key = key;
  p->value = value;

  HASH_ADD_INT(precedences, key, p);
}

bool
map_precedences_contains(enum tokenkind key) {
  struct _precedence *p;
  HASH_FIND_INT(precedences, &key, p);
  return p != NULL;
}

enum precedence
map_precedences_get(enum tokenkind key) {
  struct _precedence *p;
  HASH_FIND_INT(precedences, &key, p);
  return p->value;
}

void
map_precedences_deinit(void) {
  struct _precedence *curr, *tmp;
  HASH_ITER(hh, precedences, curr, tmp) {
    HASH_DEL(precedences, curr);
    free(curr);
  }
}
