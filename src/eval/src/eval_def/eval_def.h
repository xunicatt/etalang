#ifndef __EVAL_DEF_H__
#define __EVAL_DEF_H__
#include <lexer.h>
#include <scope.h>

struct eval {
  struct lexer *lexer;
  struct scope *scope;
};

#endif
