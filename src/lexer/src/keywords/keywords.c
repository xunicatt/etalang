
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the keywords to token type
**  File: keywords.c
*/

#include <tokens.h>
#include <keywords.h>
#include <uthash.h>

struct keyword {
  const char* key;
  enum tokenkind value;
  UT_hash_handle hh;
};

static struct keyword *keywords = NULL;

static void add(const char *key, enum tokenkind value);

void
map_keywords_init(void) {
  add("let", TLET);
  add("fn", TFUNC);
  add("if", TIF);
  add("else", TELSE);
  add("for", TFOR);
  // add("break", TBREAK);
  // add("continue", TCONTINUE);
  add("return", TRETURN);
  // add("switch", TSWITCH);
  // add("case", TCASE);
  add("true", TBOOL);
  add("false", TBOOL);
  // add("struct", TSTRUCT);
  add("extern", TEXTERN);
  add("void", TTVOID);
  add("int", TTINT);
  add("float", TTFLOAT);
  add("bool", TTBOOL);
  add("string", TTSTRING);
  add("null", TNULL);
}

static void
add(const char *key, enum tokenkind value) {
  struct keyword *k = (struct keyword*)malloc(sizeof(struct keyword));
  k->key = key;
  k->value = value;

  HASH_ADD_STR(keywords, key, k);
}

bool
map_keywords_contains(const char *key) {
  struct keyword *k;
  HASH_FIND_STR(keywords, key, k);
  return k != NULL;
}

enum tokenkind
map_keywords_get(const char *key) {
  struct keyword* k;
  HASH_FIND_STR(keywords, key, k);
  return k->value;
}

void
map_keywords_deinit(void) {
  struct keyword  *curr, *tmp;
  HASH_ITER(hh, keywords, curr, tmp) {
    HASH_DEL(keywords, curr);
    free(curr);
  }
}
