
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: An implementation of Pratt Parser
**  File: parser.c
*/

#include <precedences.h>
#include <stdio.h>
#include <tokens.h>
#include <ast.h>
#include <stdlib.h>
#include <string.h>
#include <utarray.h>
#include <parser.h>
#include <infixfns.h>
#include <prefixfns.h>
#include <lexer.h>
#include <core.h>
#include <utstring.h>

static UT_icd str_icd = {sizeof(UT_string*), NULL, NULL, NULL};
static UT_icd statement_icd = {sizeof(struct statement*), NULL, NULL, NULL};

void
error(struct parser* const self, const char* msg) {
  UT_string *errmsg = fmterror(self->lexer, lastlocation(self->lexer), msg);
  utarray_push_back(self->errors, &errmsg);
}

void
parser_init(struct parser* const self, struct lexer* const lexer) {
  self->lexer = lexer;
  utarray_new(self->errors, &str_icd);

  map_prefixfns_init();
  map_infixfns_init();
  map_precedences_init();

  map_prefixfns_add(TIDENTIFIER, pidentifierexp);
  map_prefixfns_add(TNULL, pnulllit);
  map_prefixfns_add(TINT, pintegerlit);
  map_prefixfns_add(TFLOAT, pfloatlit);
  map_prefixfns_add(TBOOL, pboollit);
  map_prefixfns_add(TSTRING, pstringlit);
  map_prefixfns_add(TNOT, pprefixexp);
  map_prefixfns_add(TSUB, pprefixexp);
  map_prefixfns_add(TOPAREN, pgroupedexp);
  map_prefixfns_add(TIF, pifexp);
  map_prefixfns_add(TFOR, pforexp);
  map_prefixfns_add(TFUNC, pfnexp);
  map_prefixfns_add(TOSQR, parraylit);
  map_prefixfns_add(TEXTERN, pexternexp);

  map_infixfns_add(TADD, pinfixexp);
  map_infixfns_add(TSUB, pinfixexp);
  map_infixfns_add(TMUL, pinfixexp);
  map_infixfns_add(TDIV, pinfixexp);
  map_infixfns_add(TEQL, pinfixexp);
  map_infixfns_add(TNEQL, pinfixexp);
  map_infixfns_add(TLES, pinfixexp);
  map_infixfns_add(TLEE, pinfixexp);
  map_infixfns_add(TGRT, pinfixexp);
  map_infixfns_add(TGRE, pinfixexp);
  map_infixfns_add(TAND, pinfixexp);
  map_infixfns_add(TOPAREN, pcallexp);
  map_infixfns_add(TASS, passignmentexp);
  map_infixfns_add(TOSQR, pindexexp);
  map_infixfns_add(TADAS, popassignmentexp);
  map_infixfns_add(TSBAS, popassignmentexp);
  map_infixfns_add(TMLAS, popassignmentexp);
  map_infixfns_add(TDVAS, popassignmentexp);
}

struct program*
parse(struct parser* const self) {
  struct program *p = (struct program*)calloc(1, sizeof(struct program));
  utarray_new(p->statements, &statement_icd);

  while(token(self->lexer) != TEOF) {
    if(lasttoken(self->lexer) == TERROR) {
      error(self, "unknown token");
      goto cleanup;
    }

    struct statement *stmt = pstatement(self);
    if(stmt == NULL) {
      goto cleanup;
    }

    #ifdef PARSER_DEBUG_ENABLE
      UT_string *debug = statement_tostr(stmt);
      printf("%s\n", utstring_body(debug));
      utstring_free(debug);
    #endif

    utarray_push_back(p->statements, &stmt);
  }

  return p;

cleanup:
  program_free(p);
  return NULL;
}

void
parser_deinit(struct parser * const self) {
  while(utarray_len(self->errors)) {
    UT_string **s = utarray_back(self->errors);
    utarray_pop_back(self->errors);
    utstring_free(*s);
  }

  utarray_free(self->errors);
  map_prefixfns_deinit();
  map_infixfns_deinit();
  map_precedences_deinit();
}
