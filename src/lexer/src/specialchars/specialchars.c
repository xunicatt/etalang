
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the special characters to token type
**  File: specialchars.c
*/

#include <stdlib.h>
#include <tokens.h>
#include <specialchars.h>
#include <uthash.h>

struct specialchar {
  int key;
  enum tokenkind value;
  UT_hash_handle hh;
};

static struct specialchar *specialchars = NULL;

static void add(const char key, enum tokenkind value);

void
map_specialchars_init(void) {
  add('(', TOPAREN);
  add(')', TCPAREN);
  add('{', TOCURLY);
  add('}', TCCURLY);
  add('[', TOSQR);
  add(']', TCSQR);
  add(',', TCOMMA);
  add(';', TSEMICOLON);
  add(':', TCOLON);
  add('>', TGRT);
  add('<', TLES);
  add('.', TDOT);
  add('+', TADD);
  add('-', TSUB);
  add('/', TDIV);
  add('*', TMUL);
  add('^', TEXP);
  add('&', TUAND);
  add('|', TUOR);
  add('%', TPER);
  add('=', TASS);
  add('~', TUNOT);
  add('!', TNOT);
}

static void
add(const char key, enum tokenkind value) {
  struct specialchar* c = (struct specialchar*)malloc(sizeof(struct specialchar));
  c->key = key;
  c->value = value;

  HASH_ADD_INT(specialchars, key, c);
}

bool
map_specialchars_contains(const char key) {
  struct specialchar* c;
  int _key = key;
  HASH_FIND_INT(specialchars, &_key, c);
  return c != NULL;
}

enum tokenkind
map_specialchars_get(const char key) {
  struct specialchar* c;
  int _key = key;
  HASH_FIND_INT(specialchars, &_key, c);
  return c->value;
}

void
map_specialchars_deinit(void) {
  struct specialchar*curr, *tmp;
  HASH_ITER(hh, specialchars, curr, tmp) {
    HASH_DEL(specialchars, curr);
    free(curr);
  }
}
