
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: core part of the parser
**  File: core.h
*/

#ifndef __PARSER_CORE_H__
#define __PARSER_CORE_H__

#include <tokens.h>
#include <ast.h>
#include <parser_def.h>
#include <utarray.h>

#define steal(x, y) do {                 \
                      (x) = (y);         \
                      (y) = NULL;        \
                    } while(0)           \

void error(struct parser* const, const char*);

struct statement* pstatement(struct parser* const);
struct statement* pletstatement(struct parser* const);
struct statement* preturnstatement(struct parser* const);
struct statement* pexpressionstatement(struct parser* const);
struct blockstatement* pblockstatement(struct parser* const);

struct expression* pexpression(struct parser* const, enum precedence);
struct expression* pprefixexp(struct parser* const);
struct expression* pinfixexp(struct parser* const, struct expression*);
struct expression* pgroupedexp(struct parser* const);
UT_array* plistexp(struct parser* const, enum tokenkind);

struct expression* pnulllit(struct parser* const);
struct expression* pintegerlit(struct parser* const);
struct expression* pfloatlit(struct parser* const);
struct expression* pboollit(struct parser* const);
struct expression* pstringlit(struct parser* const);
struct expression* parraylit(struct parser* const);

struct expression* pidentifierexp(struct parser* const);
struct expression* passignmentexp(struct parser* const, struct expression*);

struct expression* pindexexp(struct parser* const, struct expression*);
struct expression* popassignmentexp(struct parser* const, struct expression*);

struct expression* pifexp(struct parser* const);
struct expression* pforexp(struct parser* const);
struct expression* pfnexp(struct parser* const);
struct expression* pcallexp(struct parser* const, struct expression*);
struct expression* pexternexp(struct parser* const);

enum precedence ppeekprec(struct parser* const);
enum precedence pcurrprec(struct parser* const);
UT_array* pfnparam(struct parser* const);

#endif
