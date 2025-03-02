
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for the uthash hash implementation
**                for the keywords to token type
**  File: keywords.h
*/

#ifndef __LEXER_KEYWORDS_H__
#define __LEXER_KEYWORDS_H__

#include <tokens.h>
#include <stdbool.h>

void map_keywords_init(void);
bool map_keywords_contains(const char* key);
enum tokenkind map_keywords_get(const char* key);
void map_keywords_deinit(void);

#endif
