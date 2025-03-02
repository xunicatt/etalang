
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the builtin function type
**  File: builtins.h
*/

#ifndef __EVAL_BUILTINS_H__
#define __EVAL_BUILTINS_H__

#include <objects.h>
#include <stdbool.h>

void map_builtinfn_init(void);
void map_builtinfn_add(const char* name, builtinfn fn);
bool map_builtinfn_contains(const char* name);
struct object* map_builtinfn_get(const char* name);
void map_builtinfn_deinit(void);

#endif
