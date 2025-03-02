
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: runtime executer for any eta program
**  File: exec.c
*/

#include <ast.h>
#include <utstring.h>
#include <gc.h>
#include <lexer.h>
#include <parser.h>
#include <eval.h>
#include <scope.h>
#include <exec.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <objects.h>

extern bool iserr(const struct object* const);

static char*
readfile(const char *filename) {
  FILE *file = fopen(filename, "r+");
  if(file == NULL) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *content = (char*)malloc(sizeof(char) * (size + 1));
  fread(content, size, 1, file);
  fclose(file);
  content[size] = 0;
  return content;
}

static int32_t
run(const char *filename, const char *data, size_t len) {
  struct scope scope = {0};
  struct lexer lexer = {0};
  struct parser parser = {0};
  struct eval eval_ = {0};
  struct program *program = NULL;
  UT_string **error = NULL;
  struct object *result = NULL;
  int32_t ret = 0;

  gc_init();
  scope_init(&scope, NULL);
  lexer_init(&lexer, filename, data, len);
  parser_init(&parser, &lexer);
  program = parse(&parser);

  if(utarray_len(parser.errors) > 0) {
    while((error = utarray_next(parser.errors, error))) {
      printf("%s\n", utstring_body(*error));
    }

    ret = 1;
    goto cleanup;
  }

  eval_init(&eval_, &lexer, &scope);

  result = eval(&eval_, program);
  if(iserr(result)) {
    UT_string *errmsg = object_tostr(result);
    printf("%s\n", utstring_body(errmsg));
    utstring_free(errmsg);
    ret = 1;
  }
  gc_done(result);

  eval_deinit();

cleanup:
  program_free(program);
  parser_deinit(&parser);
  lexer_deinit(&lexer);
  scope_deinit(&scope);
  gc_deinit();
  return ret;
}

int32_t
exec(const char *filename) {
  char *data = readfile(filename);
  if(data == NULL) {
    fprintf(stderr, "eta: failed to open file '%s'\n", filename);
    return 1;
  }

  size_t len = strlen(data);
  int32_t ret = run(filename, data, len);
  free(data);
  return ret;
}
