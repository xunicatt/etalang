
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the token to infix parsing functions type
**  File: infixfns.h
*/

#ifndef __INFIXFNS_H__
#define __INFIXFNS_H__

#include <ast.h>
#include <parser_def.h>
#include <tokens.h>
#include <stdbool.h>

typedef struct expression*(*infixfn)(struct parser* const, struct expression*);

void map_infixfns_init(void);
void map_infixfns_add(enum tokenkind key, infixfn fn);
bool map_infixfns_contains(enum tokenkind key);
infixfn map_infixfns_get(enum tokenkind key);
void map_infixfns_deinit(void);

#endif
