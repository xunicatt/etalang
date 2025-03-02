#ifndef __PARSER_DEF_H__
#define __PARSER_DEF_H__

#include <lexer.h>
#include <utarray.h>

struct parser {
  struct lexer *lexer;
  UT_array *errors; //string_icd
};

enum precedence {
  PNONE = 0,
  PLOWEST,
  PASSIGNMENT,
  PEQUALS,
  PLESSGREATER,
  PSUM,
  PPRODUCT,
  PPREFIX,
  PCALL,
  PINDEX,
};

#endif
