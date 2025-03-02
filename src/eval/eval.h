
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Tree walking evaluator
**  File: eval.h
*/

#ifndef __EVAL_H__
#define __EVAL_H__

#include <objects.h>
#include <ast.h>
#include <scope.h>
#include <lexer.h>
#include <eval_def.h>

void eval_init(struct eval*, struct lexer*, struct scope*);
struct object* eval(struct eval*, const struct program*);
void eval_deinit(void);

#endif
