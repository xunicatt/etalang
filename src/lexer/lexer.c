
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Lexer to generate tokens from source code
**  File: lexer.c
*/

#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tokens.h>
#include <lexer.h>
#include <keywords.h>
#include <specialchars.h>
#include <utstring.h>

static bool end(const struct lexer* const);
static char currchar(const struct lexer* const);
static char peekchar(const struct lexer* const);
static void forward(struct lexer* const);
static void pin(struct lexer* const);
static void trim(struct lexer* const);
static void dropline(struct lexer* const);
static enum tokenkind _token(struct lexer* const);

#ifdef LEXER_DEBUG_ENABLE
  static void debug(struct lexer* const);
#endif

void
lexer_init(
  struct lexer* const self,
  const char* filename,
  const char* data,
  size_t len
) {
  self->filename = filename;
  self->data = data;
  self->len = len;

  map_keywords_init();
  map_specialchars_init();
}

static bool
end(const struct lexer* const self) {
  return self->loc.cursor >= self->len;
}

static char
currchar(const struct lexer* const self) {
  if(end(self)) return 0;
  return self->data[self->loc.cursor];
}

static char
peekchar(const struct lexer* const self) {
  if(self->loc.cursor + 1 >= self->len) return 0;
  return self->data[self->loc.cursor + 1];
}

static void
forward(struct lexer* const self) {
  if(!end(self) && self->loc.cursor++ && !end(self) && currchar(self) == '\n') {
    self->loc.row++;
    self->loc.linebeg = self->loc.cursor + 1;
  }
}

static void
pin(struct lexer* const self) {
  self->lastloc = self->loc;
}

static void
trim(struct lexer* const self) {
  while(!end(self) && isspace(currchar(self)))
    forward(self);
}

static void
dropline(struct lexer* const self) {
  while(!end(self) && currchar(self) != '\n')
    forward(self);
}

static enum tokenkind
_token(struct lexer* const self) {
  trim(self);

  while(currchar(self) == '#') {
    dropline(self);
    trim(self);
  }

  pin(self);

  if(end(self))
    return TEOF;

  if(isalpha(currchar(self)) || currchar(self) == '_') {
    while(!end(self) &&
      (isalpha(currchar(self)) || isdigit(currchar(self)) || currchar(self) == '_'))
      forward(self);

    size_t size = self->loc.cursor - self->lastloc.cursor + 1;
    char* identifier = (char*)malloc(sizeof(char)*size);
    assert(identifier != NULL);
    strncpy(identifier, self->data + self->lastloc.cursor, size);
    identifier[size - 1] = '\0';

    if(!map_keywords_contains(identifier)) {
      self->valuelen = size;
      self->value.valstring = identifier;
      return TIDENTIFIER;
    }

    enum tokenkind token = map_keywords_get(identifier);

    if(token == TBOOL) {
      self->value.valbool = !strcmp(identifier, "true");
    }

    free(identifier);
    return token;
  }

  if(isdigit(currchar(self))) {
    bool isfloat = false;

    while(!end(self) && (isdigit(currchar(self)) || currchar(self) == '.')) {
      if(currchar(self) == '.' && !isfloat) {
        isfloat = true;
      }

      forward(self);
    }

    size_t size = self->loc.cursor - self->lastloc.cursor + 1;
    char *value = (char*)malloc(sizeof(char)*size);
    assert(value != NULL);
    strncpy(value, self->data + self->lastloc.cursor, size);
    value[size - 1] = '\0';

    if(isfloat) {
      self->value.valfloat = atof(value);
      free(value);
      return TFLOAT;
    }

    self->value.valinteger = atoll(value);
    free(value);
    return TINT;
  }

  if(currchar(self) == '"') {
    forward(self);

    while(!end(self) && currchar(self) != '"')
      forward(self);

    size_t size = self->loc.cursor - self->lastloc.cursor;
    char *string = (char*)malloc(sizeof(char)*size);
    assert(string != NULL);

    size_t i = 0, j = self->lastloc.cursor + 1;
    while(j < (self->lastloc.cursor + size)) {
      if(self->data[j] ==  '\\' && j < (self->lastloc.cursor + size) - 1) {
        switch(self->data[j + 1]) {
          case 'n':
            string[i++] = '\n';
            j += 2;
            break;

          case 't':
            string[i++] = '\t';
            j += 2;
            break;
        }
        continue;
      }

      string[i++] = self->data[j++];
    }
    string[i] = '\0';

    forward(self);
    self->value.valstring = string;
    self->valuelen = size;
    return TSTRING;
  }

  if(map_specialchars_contains(currchar(self))) {
    enum tokenkind currtoken = map_specialchars_get(currchar(self));
    enum tokenkind finaltoken = currtoken;

    forward(self);
    char nextchar = currchar(self);

    if(map_specialchars_contains(nextchar)) {
      enum tokenkind nexttoken = map_specialchars_get(currchar(self));
      switch(currtoken) {
        case TASS:
          if (nexttoken == TASS) {
            finaltoken = TEQL;
          }
          break;

        case TUOR:
          if (nexttoken == TUOR) {
            finaltoken = TOR;
          }
          break;

        case TUAND:
          if (nexttoken == TUAND) {
            finaltoken = TAND;
          }
          break;

        case TGRT:
          if (nexttoken == TASS) {
            finaltoken = TGRE;
          }
          break;

        case TLES:
          if (nexttoken == TASS) {
            finaltoken = TLEE;
          }
          break;

        case TNOT:
          if (nexttoken == TASS) {
            finaltoken = TNEQL;
          }
          break;

        case TADD:
          if (nexttoken == TASS) {
            finaltoken = TADAS;
          }
          break;

        case TSUB:
          if (nexttoken == TASS) {
            finaltoken = TSBAS;
          }
          break;

        case TMUL:
          if (nexttoken == TASS) {
            finaltoken = TMLAS;
          }
          break;

        case TDIV:
          if (nexttoken == TASS) {
            finaltoken = TDVAS;
          }
          break;

        case TDOT: {
          if(nexttoken == TDOT && peekchar(self) == '.') {
            forward(self);
            finaltoken = TVARIADIC;
          } else {
            finaltoken = TRANGE;
          }

          break;
        }

        default:
          break;
      }

      if(currtoken != finaltoken) {
        forward(self);
        self->lasttoken = finaltoken;
      }
    }

    return finaltoken;
  }

  return TERROR;
}

void
setlocation(struct lexer* const self, const struct location loc) {
  self->loc = loc;
}

enum tokenkind
token(struct lexer* const self) {
  self->lasttoken = _token(self);
  #ifdef LEXER_DEBUG_ENABLE
    debug(self);
  #endif
  return self->lasttoken;
}

enum tokenkind
lasttoken(const struct lexer* const self) {
  return self->lasttoken;
}

enum tokenkind
peektoken(const struct lexer* const self) {
  struct lexer l = *self;
  enum tokenkind token = _token(&l);

  if(token == TIDENTIFIER || token == TSTRING) {
    free(l.value.valstring);
  }

  return token;
}

union value*
value(struct lexer* const self) {
  return &self->value;
}

struct location
location(const struct lexer* const self) {
  return self->loc;
}

struct location
lastlocation(const struct lexer* const self) {
  return self->lastloc;
}

const char*
filename(const struct lexer* const self) {
  return self->filename;
}

const char*
line(const struct lexer* const self) {
  char *newln = strchr(self->data + self->lastloc.linebeg, '\n');
  if(newln == NULL) {
    return self->data + self->lastloc.linebeg;
  }

  *newln = '\0';
  return self->data + self->lastloc.linebeg;
}

UT_string*
fmterror(struct lexer* const self, const struct location _loc, const char* msg) {
  setlocation(self, _loc);
  token(self);

  if(lasttoken(self) == TIDENTIFIER || lasttoken(self) == TSTRING) {
    free(self->value.valstring);
  }

  struct location loc = location(self);
  struct location lastloc = lastlocation(self);
  UT_string *errmsg;
  utstring_new(errmsg);

  utstring_printf(
    errmsg,
    "eta: \e[31merror in file: %s:%zu:%zu\e[0m\n",
    filename(self),
    lastloc.row + 1,
    lastloc.cursor - lastloc.linebeg + 1
  );

  utstring_printf(
    errmsg,
    "%zu | %s\n",
    lastloc.row + 1,
    line(self)
  );

  char upyarrow[loc.cursor - lastloc.cursor + 1];
  upyarrow[loc.cursor - lastloc.cursor] = '\0';
  memset(upyarrow, '^', sizeof(upyarrow) - 1);

  utstring_printf(
    errmsg,
    "%*s%*s   \e[31m%s\e[0m\n",
    (int)floor(log10(lastloc.row + 1) + 1), "",
    (int)(lastloc.cursor - lastloc.linebeg), "",
    upyarrow
  );

  utstring_printf(
    errmsg,
    "%*s%*s   \e[31m%s\e[0m",
    (int)floor(log10(lastloc.row + 1) + 1), "",
    (int)(lastloc.cursor - lastloc.linebeg), "",
    msg
  );

  return errmsg;
}

void
lexer_deinit(struct lexer* const lexer) {
  map_keywords_deinit();
  map_specialchars_deinit();
}

#ifdef LEXER_DEBUG_ENABLE
  static void debug(struct lexer* const self) {
    printf("lexer: token_type: %s", tokenname(lasttoken(self)));

    switch(lasttoken(self)) {
    case TINT:
      printf(" | value: %ld\n", value(self)->valinteger);
      break;

    case TFLOAT:
      printf(" | value: %f\n", value(self)->valfloat);
      break;

    case TBOOL:
      printf(" | value: %s\n", value(self)->valbool ? "true" : "false");
      break;

    case TIDENTIFIER:
    case TSTRING:
      printf(" | value: %s\n", value(self)->valstring);
      break;

    default:
      printf("\n");
    }
  }
#endif
