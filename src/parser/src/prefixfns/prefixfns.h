
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the token to prefix parsing functions type
**  File: prefixfns.h
*/

#ifndef __PREFIXFNS_H__
#define __PREFIXFNS_H__

#include <tokens.h>
#include <ast.h>
#include <parser_def.h>
#include <stdbool.h>

typedef struct expression*(*prefixfn)(struct parser *const);

void map_prefixfns_init(void);
void map_prefixfns_add(enum tokenkind key, prefixfn fn);
bool map_prefixfns_contains(enum tokenkind key);
prefixfn map_prefixfns_get(enum tokenkind key);
void map_prefixfns_deinit(void);

#endif
