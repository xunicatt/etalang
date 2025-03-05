
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Core part of the tree walking evaluator
**  File: core.h
*/

#include <objects.h>
#include <eval_def.h>
#include <lexer.h>
#include <ast.h>
#include <stdbool.h>

struct object* serror(const char *);
struct object* derror(struct eval*, const struct location*, struct simpleerror*);
bool iserr(const struct object* const);

struct object* estatement(struct eval*, const struct statement*);
struct object* eexpression(struct eval*, struct expression*);

struct object* blen(const UT_array*);
struct object* btypeof(const UT_array*);
struct object* btoint(const UT_array*);
struct object* btofloat(const UT_array*);
struct object* bprint(const UT_array*);
struct object* bprintln(const UT_array*);
struct object* bpush(const UT_array*);
struct object* bpop(const UT_array*);
struct object* bslice(const UT_array*);
struct object* breadint(const UT_array*);
struct object* breadfloat(const UT_array*);
struct object* breadstring(const UT_array*);

#define GETDERROR(derr, eval, loc, msg) do {                                                                \
                                  struct object *serr = serror((msg));                                      \
                                  struct object *__derr = derror((eval), (loc), &serr->simpleerror);        \
                                  gc_done(serr);                                                            \
                                  derr = __derr;                                                            \
                                } while(0)                                                                  \

#define DERROR(eval, loc, msg) do {                                                               \
                                  struct object *derr;                                            \
                                  GETDERROR(derr, eval, loc, msg);                                \
                                  return derr;                                                    \
                                } while(0)                                                        \

#define IFERROR(eval, loc, err) do {                                                                        \
                                  if(iserr((err))) {                                                        \
                                    if((err)->kind == OSIMPLEERROR) {                                       \
                                      struct object *derr = derror((eval), (loc), &(err)->simpleerror);     \
                                      gc_done((err));                                                       \
                                      return derr;                                                          \
                                    }                                                                       \
                                    return (err);                                                           \
                                  }                                                                         \
                                } while(0)                                                                  \

#define steal(x, y) do {                 \
                      (x) = (y);         \
                      (y) = NULL;        \
                    } while(0)
