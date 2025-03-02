
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the token to precedence type
**  File: precedences.h
*/

#ifndef __PRECEDENCES_H__
#define __PRECEDENCES_H__

#include <parser_def.h>
#include <tokens.h>
#include <stdbool.h>

void map_precedences_init(void);
bool map_precedences_contains(enum tokenkind key);
enum precedence map_precedences_get(enum tokenkind key);
void map_precedences_deinit(void);

#endif
