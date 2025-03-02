
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: core part of the parser
**  File: statements.h
*/

#include <parser_def.h>
#include <stdio.h>
#include <tokens.h>
#include <ast.h>
#include <stdlib.h>
#include <string.h>
#include <utarray.h>
#include <core.h>
#include <lexer.h>

static UT_icd statement_icd = {sizeof(struct statement*), NULL, NULL, NULL};

struct statement*
pletstatement(struct parser* const self) {
  struct statement *stmt = (struct statement*)calloc(1, sizeof(struct statement));
  stmt->kind = NLETSTATEMENT;

  struct letstatement *lstmt = &stmt->letstatement;
  lstmt->location = lastlocation(self->lexer);
  lstmt->kind = stmt->kind;

  if(peektoken(self->lexer) != TIDENTIFIER) {
    error(self, "expected identifier");
    goto cleanup;
  }

  token(self->lexer); //get ident
  lstmt->name = (struct identifierexp*)calloc(1, sizeof(struct identifierexp));
  lstmt->name->location = lastlocation(self->lexer);
  steal(lstmt->name->value, self->lexer->value.valstring);

  if(peektoken(self->lexer) != TASS) {
    error(self, "a variable must be initialized with a value");
    goto cleanup;
  }

  token(self->lexer); //get =
  token(self->lexer); //eat =

  lstmt->value = pexpression(self, PLOWEST);
  if(lstmt->value == NULL) {
    goto cleanup;
  }

  if(peektoken(self->lexer) == TSEMICOLON) {
    token(self->lexer); //get ;
  }

  return stmt;

cleanup:
  statement_free(stmt);
  free(stmt);
  return NULL;
}

struct blockstatement*
pblockstatement(struct parser* const self) {
  struct blockstatement *bstmt = (struct blockstatement*)calloc(1, sizeof(struct blockstatement));
  bstmt->location = location(self->lexer);
  bstmt->kind = NBLOCKSTATEMENT;

  utarray_new(bstmt->statements, &statement_icd);
  token(self->lexer); //eat {

  while(lasttoken(self->lexer) != TCCURLY) {
    if(lasttoken(self->lexer) == TEOF) {
      error(self, "expected }");
      goto cleanup;
    }

    if(lasttoken(self->lexer) == TERROR) {
      error(self, "unknown token");
      goto cleanup;
    }

    struct statement *stmt = pstatement(self);
    if(stmt == NULL) {
      goto cleanup;
    }

    utarray_push_back(bstmt->statements, &stmt);
    token(self->lexer); //fetch next token
  }

  return bstmt;

cleanup:
  blockstatement_free(bstmt);
  free(bstmt);
  return NULL;
}

struct statement*
preturnstatement(struct parser* const self) {
  struct statement *stmt = (struct statement*)calloc(1, sizeof(struct statement));
  stmt->kind = NRETURNSTATEMENT;

  struct returnstatement *rstmt = &stmt->returnstatement;
  rstmt->location = location(self->lexer);
  rstmt->kind = stmt->kind;

  token(self->lexer); //eat return

  if(lasttoken(self->lexer) == TSEMICOLON) {
    return stmt;
  }

  rstmt->value = pexpression(self, PLOWEST);
  if(rstmt->value == NULL) {
    goto cleanup;
  }

  if(peektoken(self->lexer) == TSEMICOLON) {
    token(self->lexer);
  }

  return stmt;

cleanup:
  statement_free(stmt);
  free(stmt);
  return NULL;
}

struct statement*
pexpressionstatement(struct parser* const self) {
  struct statement *stmt = (struct statement*)calloc(1, sizeof(struct statement));
  stmt->kind = NEXPRESSIONSTATEMENT;

  struct expressionstatement *estmt = &stmt->expressionstatement;
  estmt->location = location(self->lexer);
  estmt->kind = stmt->kind;

  estmt->expression = pexpression(self, PLOWEST);
  if(estmt->expression == NULL) {
    goto cleanup;
  }

  if(peektoken(self->lexer) == TSEMICOLON) {
    token(self->lexer); //get ;
  }

  return stmt;

cleanup:
  statement_free(stmt);
  free(stmt);
  return NULL;
}

struct statement*
pstatement(struct parser* const self) {
  switch(lasttoken(self->lexer)) {
  case TSEMICOLON:
    token(self->lexer);
    return NULL;

  case TLET:
    return pletstatement(self);

  case TRETURN:
    return preturnstatement(self);

  case TERROR:
    return NULL;

  default:
    return pexpressionstatement(self);
  }
}
