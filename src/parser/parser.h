
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: An implementation of Pratt Parser
**  File: parser.h
*/

#ifndef __PARSER_H__
#define __PARSER_H__

#include <tokens.h>
#include <lexer.h>
#include <ast.h>
#include <utarray.h>
#include <parser_def.h>

void parser_init(struct parser * const self, struct lexer * const lexer);
struct program* parse(struct parser * const self);
void parser_deinit(struct parser * const self);

#endif
