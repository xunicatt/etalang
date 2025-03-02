
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Lexer to generate tokens from source code
**  File: lexer.h
*/

#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdint.h>
#include <tokens.h>
#include <stddef.h>
#include <utstring.h>
#include <stdbool.h>

struct location {
  size_t cursor;
  size_t row;
  size_t linebeg;
};

union value {
  int64_t valinteger;
  double valfloat;
  char* valstring;
  bool valbool;
};

struct lexer {
  const char* filename;
  const char* data;
  size_t len;
  enum tokenkind lasttoken;
  struct location loc;
  struct location lastloc;
  union value value;
  size_t valuelen;
};

void lexer_init(struct lexer* const l, const char* filename, const char* data, size_t len);
void lexer_deinit(struct lexer* const);

void setlocation(struct lexer* const, const struct location);
enum tokenkind token(struct lexer* const);
enum tokenkind lasttoken(const struct lexer* const);
enum tokenkind peektoken(const struct lexer* const);
union value* value(struct lexer* const);
const char* line(const struct lexer* const);
struct location location(const struct lexer* const);
struct location lastlocation(const struct lexer* const);
const char* filename(const struct lexer* const);
UT_string* fmterror(struct lexer* const, const struct location, const char*);

#endif
