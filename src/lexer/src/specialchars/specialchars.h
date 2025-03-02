
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the special characters to token type
**  File: specialchars.h
*/

#ifndef __LEXER_SPCHARS_H__
#define __LEXER_SPCHARS_H__

#include <tokens.h>
#include <stdbool.h>

void map_specialchars_init(void);
bool map_specialchars_contains(const char key);
enum tokenkind map_specialchars_get(const char key);
void map_specialchars_deinit(void);

#endif
