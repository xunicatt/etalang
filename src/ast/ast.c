
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Abstract Syntax Tree implementation
**  File: ast.h
*/

#include <tokens.h>
#include <stddef.h>
#include <stdlib.h>
#include <utarray.h>
#include <utstring.h>
#include <ast.h>
#include <inttypes.h>

// NODE FREE FUNCTIONS

void
statement_free(struct statement *self) {
  if(self == NULL) return;

  switch(self->kind) {
    case NLETSTATEMENT: {
      letstatement_free(&self->letstatement);
      return;
    }

    case NRETURNSTATEMENT: {
      struct returnstatement *stmt = &self->returnstatement;
      expression_free(stmt->value);
      free(stmt->value);
      return;
    }

    case NEXPRESSIONSTATEMENT: {
      struct expressionstatement *stmt = &self->expressionstatement;
      expression_free(stmt->expression);
      free(stmt->expression);
      return;
    }

    default:
      return;
  }
}

void
blockstatement_free(struct blockstatement *self) {
  if(self == NULL) return;

  if(self->statements != NULL) {
    while(utarray_len(self->statements)) {
      struct statement **stmt = utarray_back(self->statements);
      utarray_pop_back(self->statements);
      statement_free(*stmt);
      free(*stmt);
    }

    utarray_free(self->statements);
  }
}

void
letstatement_free(struct letstatement *self) {
  if(self == NULL) return;

  if(self->name) {
    free(self->name->value);
    free(self->name);
  }

  if(self->value) {
    expression_free(self->value);
    free(self->value);
  }
}

void
expression_free(struct expression *self) {
  if(self == NULL) return;

  switch(self->kind) {
    case NIDENTIFIEREXP: {
      free(self->identifierexp.value);
      return;
    }

    case NSTRINGLIT: {
      free(self->stringlit.value);
      return;
    }

    case NARRAYLIT: {
      struct arraylit *al = &self->arraylit;
      if(al->elements != NULL) {
        while(utarray_len(al->elements)) {
          struct expression **expr = utarray_back(al->elements);
          utarray_pop_back(al->elements);
          expression_free(*expr);
          free(*expr);
        }

        utarray_free(al->elements);
      }
      return;
    }

    case NFUNCTIONLIT: {
      struct functionlit *fl = &self->functionlit;
      if(fl->parameters != NULL) {
        while(utarray_len(fl->parameters)) {
          struct identifierexp **ident = utarray_back(fl->parameters);
          utarray_pop_back(fl->parameters);
          free((*ident)->value);
          free(*ident);
        }
        utarray_free(fl->parameters);
      }
      blockstatement_free(fl->body);
      free(fl->body);
      return;
    }

    case NPREFIXEXP: {
      struct prefixexp *expr = &self->prefixexp;
      expression_free(expr->right);
      free(expr->right);
      return;
    }

    case NINFIXEXP: {
      struct infixexp *expr = &self->infixexp;
      expression_free(expr->right);
      free(expr->right);
      expression_free(expr->left);
      free(expr->left);
      return;
    }

    case NIFEXP: {
      struct ifexp *expr = &self->ifexp;
      expression_free(expr->condition);
      free(expr->condition);
      blockstatement_free(expr->consequence);
      free(expr->consequence);
      blockstatement_free(expr->alternative);
      free(expr->alternative);
      return;
    }

    case NFOREXP: {
      struct forexp *expr = &self->forexp;
      statement_free(expr->initialization);
      free(expr->initialization);
      expression_free(expr->condition);
      free(expr->condition);
      statement_free(expr->updation);
      free(expr->updation);
      blockstatement_free(expr->body);
      free(expr->body);
      return;
    }

    case NASSIGNMENTEXP: {
      struct assignmentexp *expr = &self->assignmentexp;
      expression_free(expr->left);
      free(expr->left);
      expression_free(expr->right);
      free(expr->right);
      return;
    }

    case NCALLEXP: {
      struct callexp *callexp = &self->callexp;
      expression_free(callexp->function);
      free(callexp->function);
      if(callexp->arguments != NULL) {
        while(utarray_len(callexp->arguments)) {
          struct expression **expr = utarray_back(callexp->arguments);
          utarray_pop_back(callexp->arguments);
          expression_free(*expr);
          free(*expr);
        }

        utarray_free(callexp->arguments);
      }
      return;
    }

    case NINDEXEXP: {
      struct indexexp *expr = &self->indexexp;
      expression_free(expr->left);
      free(expr->left);
      expression_free(expr->index);
      free(expr->index);
      return;
    }

    case NOPASSIGNMENTEXP: {
      struct opassignmentexp *expr = &self->opassignmentexp;
      expression_free(expr->left);
      free(expr->left);
      expression_free(expr->right);
      free(expr->right);
      return;
    }

    case NEXTERNEXP: {
      struct externexp *expr = &self->externexp;
      free(expr->identifier);
      if(expr->argumenttypes != NULL) {
        utarray_free(expr->argumenttypes);
      }
    }

    default:
      return;
  }
}

void
program_free(struct program *self) {
  if(self == NULL) return;

  if(self->statements) {
    while(utarray_len(self->statements)) {
      struct statement **stmt = utarray_back(self->statements);
      utarray_pop_back(self->statements);
      statement_free(*stmt);
      free(*stmt);
    }

    utarray_free(self->statements);
  }

  free(self);
}

// NODE TOSTR FUNCTIONS

#ifdef PARSER_DEBUG_ENABLE
  UT_string*
  expression_tostr(const struct expression* const self) {
    UT_string* result;
    utstring_new(result);

    if(self == NULL) {
      utstring_printf(result, "nil");
      return result;
    }

    switch(self->kind) {
      case NIDENTIFIEREXP: {
        utstring_printf(
          result,
          "{identifier: %s}",
          self->identifierexp.value
        );
        return result;
      }

      case NINTEGERLIT: {
        utstring_printf(
          result,
          "{integer: %" PRId64 "}",
          self->integerlit.value
        );
        return result;
      }

      case NFLOATLIT: {
        utstring_printf(
          result,
          "{float: %f}",
          self->floatlit.value
        );
        return result;
      }

      case NBOOLLIT: {
        utstring_printf(
          result,
          "{bool: %s}",
          self->boollit.value ? "true" : "false"
        );
        return result;
      }

      case NSTRINGLIT: {
        utstring_printf(
          result,
          "{string: %s}",
          self->stringlit.value
        );
        return result;
      }

      case NARRAYLIT: {
        utstring_printf(result, "{array: [");
        for(size_t i = 0; i < utarray_len(self->arraylit.elements); i++) {
          struct expression **expr = utarray_eltptr(self->arraylit.elements, i);
          UT_string *sub = expression_tostr(*expr);
          utstring_printf(
            result,
            "%s%s",
            utstring_body(sub),
            i < utarray_len(self->arraylit.elements) - 1 ? ", " : ""
          );
          utstring_free(sub);
        }
        utstring_printf(result, "]}");
        return result;
      }

      case NFUNCTIONLIT: {
        utstring_printf(result, "{function: {parameters: [");
        for(size_t i = 0; i < utarray_len(self->functionlit.parameters); i++) {
          struct identifierexp **expr = utarray_eltptr(self->functionlit.parameters, i);
          utstring_printf(
            result,
            "{identifier: %s}%s",
            (*expr)->value,
            i < utarray_len(self->functionlit.parameters) - 1 ? ", " : ""
          );
        }

        UT_string *sub = blockstatement_tostr(self->functionlit.body);
        utstring_printf(result, "], body: %s}}", utstring_body(sub));
        utstring_free(sub);
        return result;
      }

      case NPREFIXEXP: {
        UT_string *sub = expression_tostr(self->prefixexp.right);
        utstring_printf(
          result,
          "{prefix: {operator: %s, right: %s}",
          tokenname(self->prefixexp.operator),
          utstring_body(sub)
        );
        utstring_free(sub);
        return result;
      }

      case NINFIXEXP: {
        UT_string *lsub = expression_tostr(self->infixexp.left);
        UT_string *rsub = expression_tostr(self->infixexp.right);
        utstring_printf(
          result,
          "{infix: {operator: %s, left: %s, right: %s}}",
          tokenname(self->infixexp.operator),
          utstring_body(lsub),
          utstring_body(rsub)
        );
        utstring_free(lsub);
        utstring_free(rsub);
        return result;
      }

      case NIFEXP: {
        UT_string *condsub = expression_tostr(self->ifexp.condition);
        UT_string *conssub = blockstatement_tostr(self->ifexp.consequence);
        UT_string *altrsub = blockstatement_tostr(self->ifexp.alternative);
        utstring_printf(
          result,
          "{branch: {condition: %s, if: %s, else: %s}",
          utstring_body(condsub),
          utstring_body(conssub),
          utstring_body(altrsub)
        );
        utstring_free(condsub);
        utstring_free(conssub);
        utstring_free(altrsub);
        return result;
      }

      case NFOREXP: {
        UT_string *initsub = statement_tostr(self->forexp.initialization);
        UT_string *condsub = expression_tostr(self->forexp.condition);
        UT_string *updtsub = statement_tostr(self->forexp.updation);
        UT_string *bodysub = blockstatement_tostr(self->forexp.body);

        utstring_printf(
          result,
          "{loop : {init: %s, condition: %s, post: %s, body: %s}}",
          utstring_body(initsub),
          utstring_body(condsub),
          utstring_body(updtsub),
          utstring_body(bodysub)
        );

        utstring_free(initsub);
        utstring_free(condsub);
        utstring_free(updtsub);
        utstring_free(bodysub);

        return result;
      }

      case NASSIGNMENTEXP: {
        UT_string *lsub = expression_tostr(self->assignmentexp.left);
        UT_string *rsub = expression_tostr(self->assignmentexp.right);

        utstring_printf(
          result,
          "{assignment: {left: %s, right: %s}}",
          utstring_body(lsub),
          utstring_body(rsub)
        );

        utstring_free(lsub);
        utstring_free(rsub);

        return result;
      }

      case NCALLEXP: {
        UT_string *fnsub = expression_tostr(self->callexp.function);
        utstring_printf(
          result,
          "{call: {function: %s, arguments: [",
          utstring_body(fnsub)
        );
        utstring_free(fnsub);

        for(size_t i = 0; i < utarray_len(self->callexp.arguments); i++) {
          struct expression **expr = utarray_eltptr(self->callexp.arguments, i);
          UT_string *sub = expression_tostr(*expr);
          utstring_printf(
            result,
            "%s%s",
            utstring_body(sub),
            i < utarray_len(self->callexp.arguments) - 1 ? ", " : ""
          );
          utstring_free(sub);
        }

        utstring_printf(result, "]}}");
        return result;
      }

      case NINDEXEXP: {
        UT_string *lsub = expression_tostr(self->indexexp.left);
        UT_string *isub = expression_tostr(self->indexexp.index);

        utstring_printf(
          result,
          "{subscript: {left: %s, index: %s}}",
          utstring_body(lsub),
          utstring_body(isub)
        );

        utstring_free(lsub);
        utstring_free(isub);
        return result;
      }

      case NOPASSIGNMENTEXP: {
        UT_string *lsub = expression_tostr(self->opassignmentexp.left);
        UT_string *rsub = expression_tostr(self->opassignmentexp.right);

        utstring_printf(
          result,
          "{op-assignment: {operator: %s, left: %s, right: %s}}",
          tokenname(self->opassignmentexp.operator),
          utstring_body(lsub),
          utstring_body(rsub)
        );

        utstring_free(lsub);
        utstring_free(rsub);

        return result;
      }

      case NEXTERNEXP: {
        UT_string *argumenttypestr;
        utstring_new(argumenttypestr);
        utstring_printf(argumenttypestr, "[");
        for(size_t i = 0; i < utarray_len(self->externexp.argumenttypes); i++) {
          enum tokenkind type = *(enum tokenkind*)utarray_eltptr(self->externexp.argumenttypes, i);
          utstring_printf(
            argumenttypestr,
            "%s%s",
            tokenname(type),
            i < utarray_len(self->externexp.argumenttypes) - 1 ? ", " : ""
          );
        }
        utstring_printf(argumenttypestr, "]");
        utstring_printf(
          result,
          "{extern: {function-name: %s, return-type: %s, argument-type: %s}}",
          self->externexp.identifier,
          tokenname(self->externexp.returntype),
          utstring_body(argumenttypestr)
        );
        utstring_free(argumenttypestr);
        return result;
      }

      default:
        utstring_printf(result, "{none}");
        return result;
    }
  }

  UT_string*
  blockstatement_tostr(const struct blockstatement* const self) {
    UT_string *result;
    utstring_new(result);

    if(self == NULL) {
      utstring_printf(result, "nil");
      return result;
    }

    utstring_printf(result, "{block: [");
    for(size_t i = 0; i < utarray_len(self->statements); i++) {
      struct statement **stmt = utarray_eltptr(self->statements, i);
      UT_string *sub = statement_tostr(*stmt);
      utstring_printf(
        result,
        "%s%s",
        utstring_body(sub),
        i < utarray_len(self->statements) - 1 ? ", " : ""
      );
      utstring_free(sub);
    }

    utstring_printf(result, "]}");
    return result;
  }


  UT_string*
  letstatement_tostr(const struct letstatement* const self) {
    UT_string *result;
    utstring_new(result);

    UT_string *sub = expression_tostr(self->value);
    utstring_printf(
      result,
      "{let: {identifier: %s, value: %s}}",
      self->name->value,
      utstring_body(sub)
    );

    utstring_free(sub);
    return result;
  }

  UT_string*
  statement_tostr(const struct statement* const self) {
    UT_string *result;
    utstring_new(result);

    if(self == NULL) {
      utstring_printf(result, "nil");
      return result;
    }

    switch(self->kind) {
      case NLETSTATEMENT:
        utstring_free(result);
        return letstatement_tostr(&self->letstatement);

      case NRETURNSTATEMENT: {
        UT_string *sub = expression_tostr(self->returnstatement.value);
        utstring_printf(result, "{return: %s}", utstring_body(sub));
        utstring_free(sub);
        return result;
      }

      case NEXPRESSIONSTATEMENT: {
        utstring_free(result);
        return expression_tostr(self->expressionstatement.expression);
      }

      default:
        return result;
    }
  }

  UT_string*
  program_tostr(const struct program* const self) {
    UT_string *result;
    utstring_new(result);

    if(self == NULL) {
      utstring_printf(result, "nil");
      return result;
    }

    utstring_printf(result, "{program: [");

    for(size_t i = 0; i < utarray_len(self->statements); i++) {
      struct statement **stmt = utarray_eltptr(self->statements, i);
      UT_string *sub = statement_tostr(*stmt);
      utstring_printf(
        result,
        "%s%s",
        utstring_body(sub),
        i < utarray_len(self->statements) - 1 ? ", " : ""
      );
      utstring_free(sub);
    }

    utstring_printf(result, "]}");
    return result;
  }
#endif
