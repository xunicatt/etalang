
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: core part of the parser
**  File: expressions.h
*/

#include <utstring.h>
#include <precedences.h>
#include <stdio.h>
#include <tokens.h>
#include <ast.h>
#include <stdlib.h>
#include <string.h>
#include <utarray.h>
#include <core.h>
#include <lexer.h>
#include <prefixfns.h>
#include <infixfns.h>
#include <stdbool.h>

static UT_icd expression_icd = {sizeof(struct expression*), NULL, NULL, NULL};
static UT_icd identifierexp_icd = {sizeof(struct identifierexp*), NULL, NULL, NULL};
static UT_icd tokenkind_icd = {sizeof(enum tokenkind), NULL, NULL, NULL};

struct expression*
pexpression(struct parser* const self, enum precedence p) {
  if(!map_prefixfns_contains(lasttoken(self->lexer))) {
    error(self, "no prefix parse function");
    return NULL;
  }

  prefixfn prefixfn = map_prefixfns_get(lasttoken(self->lexer));
  struct expression *left = prefixfn(self);
  if(left == NULL) {
    return NULL;
  }

  while(peektoken(self->lexer) != TSEMICOLON && p < ppeekprec(self)) {
    if(!map_infixfns_contains(peektoken(self->lexer))) {
      return left;
    }

    infixfn infixfn = map_infixfns_get(token(self->lexer));
    struct expression *nleft = infixfn(self, left);
    if(nleft == NULL) {
      expression_free(left);
      free(left);
      return NULL;
    }
    left = nleft;
  }

  return left;
}

struct expression*
pprefixexp(struct parser* const self) {
  struct expression *expr = (struct expression*)calloc(1, sizeof(struct expression));
  expr->kind= NPREFIXEXP;

  struct prefixexp *pexpr = &expr->prefixexp;
  pexpr->location = lastlocation(self->lexer);
  pexpr->kind = expr->kind;
  pexpr->operator = lasttoken(self->lexer);

  token(self->lexer); //eat operator
  pexpr->right = pexpression(self, PPREFIX);
  if(pexpr->right == NULL) {
    goto cleanup;
  }

  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pinfixexp(struct parser* const self, struct expression* left) {
  struct expression *expr = (struct expression*)calloc(1, sizeof(struct expression));
  expr->kind = NINFIXEXP;

  struct infixexp *iexpr = &expr->infixexp;
  iexpr->location = lastlocation(self->lexer);
  iexpr->kind = expr->kind;
  iexpr->operator = lasttoken(self->lexer);

  enum precedence p = pcurrprec(self);
  token(self->lexer); //eat operator
  iexpr->right = pexpression(self, p);
  if(iexpr->right == NULL) {
    goto cleanup;
  }

  iexpr->left = left;
  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pgroupedexp(struct parser* const self) {
  token(self->lexer); //eat (
  struct expression *expr = pexpression(self, PLOWEST);

  if(peektoken(self->lexer) == TCPAREN) {
    token(self->lexer); //get )
    return expr;
  }

  error(self, "expected )");
  expression_free(expr);
  free(expr);
  return NULL;
}

UT_array*
plistexp(struct parser* const self, enum tokenkind end) {
  UT_array *list;
  utarray_new(list, &expression_icd);

  if(peektoken(self->lexer) == end) {
    token(self->lexer); //get end
    return list;
  }

  token(self->lexer); //fetch token
  while(true) {
    struct expression *expr = pexpression(self, PLOWEST);
    if(expr == NULL) {
      goto cleanup;
    }
    utarray_push_back(list, &expr);

    if(token(self->lexer) == end) {
      break;
    }

    if(lasttoken(self->lexer) != TCOMMA) {
      error(self, "expected , or )");
      goto cleanup;
    }

    token(self->lexer);
  }

  return list;

cleanup:
  while(utarray_len(list)) {
    struct expression **expr = utarray_back(list);
    utarray_pop_back(list);
    expression_free(*expr);
    free(*expr);
  }
  utarray_free(list);
  return NULL;
}

struct expression*
pnulllit(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NNULLLIT;
  expr->nulllit.kind = expr->kind;
  expr->nulllit.location = lastlocation(self->lexer);
  return expr;
}

struct expression*
pintegerlit(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NINTEGERLIT;

  struct integerlit *ilit = &expr->integerlit;
  ilit->location = lastlocation(self->lexer);
  ilit->kind = expr->kind;
  ilit->value = value(self->lexer)->valinteger;

  return expr;
}

struct expression*
pfloatlit(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NFLOATLIT;

  struct floatlit *flit = &expr->floatlit;
  flit->location = lastlocation(self->lexer);
  flit->kind = expr->kind;
  flit->value = value(self->lexer)->valfloat;

  return expr;
}

struct expression*
pboollit(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NBOOLLIT;

  struct boollit *blit = &expr->boollit;
  blit->location = lastlocation(self->lexer);
  blit->kind = expr->kind;
  blit->value = value(self->lexer)->valbool;

  return expr;
}

struct expression*
pstringlit(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NSTRINGLIT;

  struct stringlit *blit = &expr->stringlit;
  blit->location = lastlocation(self->lexer);
  blit->kind = expr->kind;
  steal(blit->value, self->lexer->value.valstring);

  return expr;
}

struct expression*
parraylit(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NARRAYLIT;

  struct arraylit *alit = &expr->arraylit;
  alit->location = lastlocation(self->lexer);
  alit->kind = expr->kind;
  alit->elements = plistexp(self, TCSQR);
  if(alit->elements == NULL) {
    goto cleanup;
  }

  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pidentifierexp(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NIDENTIFIEREXP;

  struct identifierexp *identexp = &expr->identifierexp;
  identexp->location = lastlocation(self->lexer);
  identexp->kind = expr->kind;
  steal(identexp->value, self->lexer->value.valstring);

  return expr;
}

struct expression*
passignmentexp(struct parser* const self, struct expression* left) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NASSIGNMENTEXP;

  struct assignmentexp *aexpr = &expr->assignmentexp;
  aexpr->location = lastlocation(self->lexer);
  aexpr->kind = expr->kind;

  token(self->lexer); //eat =
  aexpr->right = pexpression(self, PLOWEST);
  if(aexpr->right == NULL) {
    goto cleanup;
  }

  aexpr->left = left;
  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pindexexp(struct parser* const self, struct expression* left) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NINDEXEXP;

  struct indexexp *indexexp = &expr->indexexp;
  indexexp->location = location(self->lexer);
  indexexp->kind = expr->kind;

  token(self->lexer); //eat [
  indexexp->index = pexpression(self, PLOWEST);
  if(indexexp->index == NULL) {
    goto cleanup;
  }

  if(peektoken(self->lexer) != TCSQR) {
    error(self, "expected ]");
    goto cleanup;
  }

  token(self->lexer); //get ]
  indexexp->left = left;
  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
popassignmentexp(struct parser* const self, struct expression* left) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NOPASSIGNMENTEXP;

  struct opassignmentexp *opexpr = &expr->opassignmentexp;
  opexpr->location = lastlocation(self->lexer);
  opexpr->kind = expr->kind;

  switch(lasttoken(self->lexer)) {
    case TADAS:
      opexpr->operator = TADD;
      break;

    case TSBAS:
      opexpr->operator = TSUB;
      break;

    case TMLAS:
      opexpr->operator = TMUL;
      break;

    case TDVAS:
      opexpr->operator = TDIV;
      break;

    default:
      opexpr->operator = TNONE;
  }

  token(self->lexer); //eat operator
  opexpr->right = pexpression(self, PLOWEST);
  if(opexpr->right == NULL) {
    goto cleanup;
  }

  opexpr->left = left;
  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pifexp(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NIFEXP;

  struct ifexp *ifexp = &expr->ifexp;
  ifexp->location = location(self->lexer);
  ifexp->kind = expr->kind;

  if(peektoken(self->lexer) != TOPAREN) {
    error(self, "expected (");
    goto cleanup;
  }

  token(self->lexer); //get (
  ifexp->condition = pexpression(self, PLOWEST);
  if(ifexp->condition == NULL) {
    goto cleanup;
  }

  if(lasttoken(self->lexer) != TCPAREN) {
    error(self, "expected )");
    goto cleanup;
  }

  if(peektoken(self->lexer) != TOCURLY) {
    error(self, "expected {");
    goto cleanup;
  }

  token(self->lexer); //get {
  ifexp->consequence = pblockstatement(self);
  if(ifexp->consequence == NULL) {
    goto cleanup;
  }

  if(peektoken(self->lexer) != TELSE) {
    return expr;
  }

  token(self->lexer); //get else
  if(peektoken(self->lexer) != TOCURLY) {
    error(self, "expected {");
    goto cleanup;
  }

  token(self->lexer); //get {
  ifexp->alternative = pblockstatement(self);
  if(ifexp->alternative == NULL) {
    goto cleanup;
  }

  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pforexp(struct parser* const self) {
  struct expression *expr = (struct expression *)calloc(1, sizeof(struct expression));
  expr->kind = NFOREXP;

  struct forexp *forexp = &expr->forexp;
  forexp->location = location(self->lexer);
  forexp->kind = expr->kind;

  if(peektoken(self->lexer) != TOPAREN) {
    error(self, "expected (");
    goto cleanup;
  }

  token(self->lexer); //get (

  //init
  if(peektoken(self->lexer) != TSEMICOLON) {
    token(self->lexer); //fetch token
    forexp->initialization = pstatement(self);
    if(forexp->initialization == NULL) {
      goto cleanup;
    }
  } else {
    token(self->lexer); //get ;
  }

  //cond
  if(peektoken(self->lexer) != TSEMICOLON) {
    token(self->lexer); //fetch token
    forexp->condition = pexpression(self, PLOWEST);
    if(forexp->condition == NULL) {
      goto cleanup;
    }
  }
  token(self->lexer);

  //updation
  if(peektoken(self->lexer) != TCPAREN) {
    token(self->lexer); //fetch token
    forexp->updation = pstatement(self);
    if(forexp->updation == NULL) {
      goto cleanup;
    }
  }

  if(peektoken(self->lexer) != TCPAREN) {
    error(self, "expected )");
    goto cleanup;
  }

  token(self->lexer); //get )
  if(peektoken(self->lexer) != TOCURLY) {
    error(self, "expected {");
    goto cleanup;
  }

  token(self->lexer); //get {
  forexp->body = pblockstatement(self);
  if(forexp->body == NULL) {
    goto cleanup;
  }

  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pfnexp(struct parser* const self) {
  struct expression *expr = (struct expression*)calloc(1, sizeof(struct expression));
  expr->kind = NFUNCTIONLIT;

  struct functionlit *fnlit = &expr->functionlit;
  fnlit->location = location(self->lexer);
  fnlit->kind = expr->kind;

  if(peektoken(self->lexer) != TOPAREN) {
    error(self, "expected (");
    goto cleanup;
  }

  token(self->lexer); //get (
  fnlit->parameters = pfnparam(self);
  if(fnlit->parameters == NULL) {
    goto cleanup;
  }

  if(peektoken(self->lexer) != TOCURLY) {
    error(self, "expected {");
    goto cleanup;
  }

  token(self->lexer);
  fnlit->body = pblockstatement(self);
  if(fnlit->body == NULL) {
    goto cleanup;
  }

  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

struct expression*
pcallexp(struct parser* const self, struct expression* left) {
  struct expression *expr = (struct expression*)calloc(1, sizeof(struct expression));
  expr->kind = NCALLEXP;

  struct callexp *callexp = &expr->callexp;
  callexp->location = lastlocation(self->lexer);
  callexp->kind = expr->kind;
  callexp->arguments = plistexp(self, TCPAREN);
  if(callexp->arguments == NULL) {
    goto cleanup;
  }

  callexp->function = left;
  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

UT_array*
pfnparam(struct parser* const self) {
  UT_array *identifiers;
  utarray_new(identifiers, &identifierexp_icd);

  if(peektoken(self->lexer) == TCPAREN) {
    token(self->lexer); //get )
    return identifiers;
  }

  token(self->lexer); //fetch token
  while(true) {
    if(lasttoken(self->lexer) != TIDENTIFIER) {
      error(self, "expected an identifier");
      goto cleanup;
    }

    struct identifierexp *identifier = (struct identifierexp*)calloc(1, sizeof(struct identifierexp));
    identifier->location = lastlocation(self->lexer);
    identifier->kind = NIDENTIFIEREXP;
    steal(identifier->value, self->lexer->value.valstring);
    utarray_push_back(identifiers, &identifier);

    token(self->lexer);
    if(lasttoken(self->lexer) == TCPAREN) {
      break;
    }

    if(lasttoken(self->lexer) != TCOMMA) {
      error(self, "expected , or )");
      goto cleanup;
    }

    token(self->lexer); //fetch token
  }

  return identifiers;

cleanup:
  while(utarray_len(identifiers)) {
    struct identifierexp **identifier = utarray_back(identifiers);
    utarray_pop_back(identifiers);
    free((*identifier)->value);
    free(*identifier);
  }
  utarray_free(identifiers);
  return NULL;
}

struct expression*
pexternexp(struct parser* const self) {
  if(peektoken(self->lexer) != TFUNC) {
    error(self, "expected function");
    return NULL;
  }

  token(self->lexer); //get fn
  if(peektoken(self->lexer) != TIDENTIFIER) {
    error(self, "expectted identifier");
    return NULL;
  }

  token(self->lexer); //get identifier
  struct expression *expr = (struct expression*)calloc(1, sizeof(struct expression));
  expr->kind = NEXTERNEXP;
  steal(expr->externexp.identifier, self->lexer->value.valstring);
  utarray_new(expr->externexp.argumenttypes, &tokenkind_icd);

  if(peektoken(self->lexer) != TOPAREN) {
    error(self, "expected (");
    goto cleanup;
  }

  token(self->lexer); //get (
  if(peektoken(self->lexer) != TCPAREN) {
    while(true) {
      enum tokenkind type = peektoken(self->lexer);
      if(type != TTINT && type != TTFLOAT && type != TTBOOL && type != TTSTRING) {
        error(self, "expected a type");
        goto cleanup;
      }

      token(self->lexer); //get the type
      utarray_push_back(expr->externexp.argumenttypes, &type);

      token(self->lexer); //fetch next token
      if(lasttoken(self->lexer) == TCPAREN) {
        break;
      }

      if(lasttoken(self->lexer) != TCOMMA) {
        error(self, "expected , or )");
        goto cleanup;
      }
    }
  } else {
    token(self->lexer); //get )
  }

  if(peektoken(self->lexer) != TCOLON) {
    error(self, "expected :");
    goto cleanup;
  }

  token(self->lexer); //get :
  enum tokenkind type = peektoken(self->lexer);
  if(type != TTINT && type != TTFLOAT && type != TTBOOL && type != TTSTRING) {
    error(self, "expected a return type");
    goto cleanup;
  }

  token(self->lexer);
  expr->externexp.returntype = type;

  return expr;

cleanup:
  expression_free(expr);
  free(expr);
  return NULL;
}

enum precedence
ppeekprec(struct parser* const self) {
  enum tokenkind t = peektoken(self->lexer);
  if(!map_precedences_contains(t)) {
    return PLOWEST;
  }

  return map_precedences_get(t);
}

enum precedence
pcurrprec(struct parser* const self) {
  enum tokenkind t = lasttoken(self->lexer);
  if(!map_precedences_contains(t)) {
    return PLOWEST;
  }

  return map_precedences_get(t);
}
