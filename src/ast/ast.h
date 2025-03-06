
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Abstract Syntax Tree implementation
**  File: ast.h
*/

#ifndef __AST_H__
#define __AST_H__

#include <utstring.h>
#include <tokens.h>
#include <stdint.h>
#include <utarray.h>
#include <lexer.h>

enum nodekind {
  NNONE = 0,
  NSTATEMENT,
  NEXPRESSION,
  NIDENTIFIEREXP,
  NNULLLIT,
  NINTEGERLIT,
  NFLOATLIT,
  NBOOLLIT,
  NSTRINGLIT,
  NARRAYLIT,
  NFUNCTIONLIT,
  NPREFIXEXP,
  NINFIXEXP,
  NIFEXP,
  NFOREXP,
  NASSIGNMENTEXP,
  NCALLEXP,
  NINDEXEXP,
  NOPASSIGNMENTEXP,
  NEXTERNEXP,
  NLETSTATEMENT,
  NBLOCKSTATEMENT,
  NRETURNSTATEMENT,
  NEXPRESSIONSTATEMENT,
};

struct statement;
struct expression;

struct identifierexp {
  enum nodekind kind;
  struct location location;
  char *value;
};

//====================================================
// LITERALS
struct nulllit {
  enum nodekind kind;
  struct location location;
};

struct integerlit {
  enum nodekind kind;
  struct location location;
  int64_t value;
};

struct floatlit {
  enum nodekind kind;
  struct location location;
  double value;
};

struct boollit {
  enum nodekind kind;
  struct location location;
  bool value;
};

struct stringlit {
  enum nodekind kind;
  struct location location;
  char *value;
};

struct arraylit {
  enum nodekind kind;
  struct location location;
  UT_array *elements; //expression_icd
};

struct functionlit {
  enum nodekind kind;
  struct location location;
  UT_array *parameters; //identifier_icd
  struct blockstatement *body;
};

//====================================================
// EXPRESSIONS
struct prefixexp {
  enum nodekind kind;
  struct location location;
  enum tokenkind operator;
  struct expression *right;
};

struct infixexp {
  enum nodekind kind;
  struct location location;
  enum tokenkind operator;
  struct expression *left;
  struct expression *right;
};

struct ifexp {
  enum nodekind kind;
  struct location location;
  struct expression *condition;
  struct blockstatement *consequence;
  struct blockstatement *alternative;
};

struct forexp {
  enum nodekind kind;
  struct location location;
  struct statement *initialization;
  struct expression *condition;
  struct statement *updation;
  struct blockstatement *body;
};

struct assignmentexp {
  enum nodekind kind;
  struct location location;
  struct expression *left;
  struct expression *right;
};

struct callexp {
  enum nodekind kind;
  struct location location;
  struct expression *function;
  UT_array *arguments; //expression_icd
};

struct indexexp {
  enum nodekind kind;
  struct location location;
  struct expression *left;
  struct expression *index;
};

struct opassignmentexp {
  enum nodekind kind;
  struct location location;
  enum tokenkind operator;
  struct expression *left;
  struct expression *right;
};

struct externexp {
  enum nodekind kind;
  struct location location;
  enum tokenkind returntype;
  char *libname;
  struct location libloc;
  char *funcname;
  struct location funcloc;
  UT_array *argumenttypes; //token_icd
};

struct expression {
  enum nodekind kind;
  union {
    struct identifierexp identifierexp;
    struct nulllit nulllit;
    struct integerlit integerlit;
    struct floatlit floatlit;
    struct boollit boollit;
    struct stringlit stringlit;
    struct arraylit arraylit;
    struct functionlit functionlit;
    struct prefixexp prefixexp;
    struct infixexp infixexp;
    struct ifexp ifexp;
    struct forexp forexp;
    struct assignmentexp assignmentexp;
    struct callexp callexp;
    struct indexexp indexexp;
    struct opassignmentexp opassignmentexp;
    struct externexp externexp;
  };
};

#ifdef PARSER_DEBUG_ENABLE
  UT_string* expression_tostr(const struct expression *const);
#endif

void expression_free(struct expression *);

//====================================================
// STATEMENTS
struct letstatement {
  enum nodekind kind;
  struct location location;
  struct identifierexp *name;
  struct expression *value;
};

#ifdef PARSER_DEBUG_ENABLE
  UT_string* letstatement_tostr(const struct letstatement *const);
#endif

void letstatement_free(struct letstatement *);

struct blockstatement {
  enum nodekind kind;
  struct location location;
  UT_array *statements; //statement_icd
};

#ifdef PARSER_DEBUG_ENABLE
  UT_string* blockstatement_tostr(const struct blockstatement *const);
#endif

void blockstatement_free(struct blockstatement *);

struct returnstatement {
  enum nodekind kind;
  struct location location;
  struct expression *value;
};

struct expressionstatement {
  enum nodekind kind;
  struct location location;
  struct expression *expression;
};

struct statement {
  enum nodekind kind;
  union {
    struct letstatement letstatement;
    struct returnstatement returnstatement;
    struct expressionstatement expressionstatement;
  };
};

#ifdef PARSER_DEBUG_ENABLE
  UT_string* statement_tostr(const struct statement *const);
#endif

void statement_free(struct statement *);

struct program {
  UT_array *statements; //statement_icd
};

#ifdef PARSER_DEBUG_ENABLE
  UT_string* program_tostr(const struct program *const);
#endif

void program_free(struct program *);

#endif
