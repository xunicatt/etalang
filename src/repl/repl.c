
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: REPL for eta
**  File: repl.c
*/

#include <scope.h>
#include <ast.h>
#include <utarray.h>
#include <lexer.h>
#include <tokens.h>
#include <utstring.h>
#include <repl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parser.h>
#include <objects.h>
#include <gc.h>
#include <eval.h>
#include <dlfcn.h>
#include <ffi.h>

#ifdef __linux__
  #define LIB_C "libc.so.6"
#elif __APPLE__
  #define LIB_C "libSystem.B.dylib"
#else
  #error "platform not supported"
#endif

#define HELPER "etalang repl -- type '.help' for help"
#define PROMPT ">> "
#define VERSION_STR "v0.0.1"
#define HELP_MSG ".help  --- help\n" \
                ".clear --- clear the terminal\n" \
                ".ver   --- shows the eta version\n" \
                ".gc    --- garbage collector\n" \
                ".exit  --- exits the repl\n" \

struct object *OBJECT_NULL = &(struct object){.kind = ONULL};
struct object *OBJECT_FALSE = &(struct object){.kind = OBOOL, .boolobj = {false}};
struct object *OBJECT_TRUE = &(struct object){.kind = OBOOL, .boolobj = {true}};
void *lib_c_handle = NULL;

void
repl() {
  // [TODO]: Handling loading of libC library
  if((lib_c_handle = dlopen(LIB_C, RTLD_LAZY)) == NULL) {
    fprintf(stderr, "failed to load '" LIB_C "'\n");
  }

  printf("\e[32m" HELPER "\n");
  printf(VERSION_STR "\e[0m\n");
  UT_string* line;
  char c;
  struct scope scope = {0};

  gc_init();
  scope_init(&scope, NULL);

  while(true) {
    utstring_new(line);
    printf(PROMPT);

    while((c = getchar()) != '\n') {
      utstring_printf(line, "%c", c);
    }

    if(utstring_len(line) > 0 && utstring_body(line)[0] == '.') {
      const char* command = utstring_body(line);
      if(!strcmp(command, ".help")) {
        printf("\e[32m" HELP_MSG "\e[0m");
        utstring_free(line);
        continue;;
      }

      if(!strcmp(command, ".clear")) {
        printf("\033c");
        utstring_free(line);
        continue;;
      }

      if(!strcmp(command, ".ver")) {
        printf(VERSION_STR "\n");
        utstring_free(line);
        continue;;
      }

      if(!strcmp(command, ".gc")) {
        printf("before cleanup:\n\tobjects: %zu\n", gc_active_objs());
        gc_cleanup();
        printf("after cleanup:\n\tobjects: %zu\n", gc_active_objs());
        utstring_free(line);
        continue;
      }

      if(!strcmp(command, ".exit")) {
        utstring_free(line);
        scope_deinit(&scope);
        gc_deinit();
        return;
      }

      printf("\e[31munknown command: %s\e[0m\n", command);
      utstring_free(line);
      continue;;
    }

    struct lexer lexer = {0};
    struct parser parser = {0};
    struct eval _eval = {0};
    lexer_init(&lexer, "repl", utstring_body(line), utstring_len(line));
    parser_init(&parser, &lexer);
    struct program *program = parse(&parser);

    if(utarray_len(parser.errors) > 0) {
      UT_string **error = NULL;
      while((error = utarray_next(parser.errors, error))) {
        printf("%s\n", utstring_body(*error));
      }

      goto cleanup;
    }

    eval_init(&_eval, &lexer, &scope);
    struct object *result = eval(&_eval, program);
    UT_string *resultstr = object_tostr(result);

    printf("%s\n", utstring_body(resultstr));
    utstring_free(resultstr);

    gc_done(result);
    eval_deinit();

  cleanup:
    program_free(program);
    parser_deinit(&parser);
    lexer_deinit(&lexer);
    utstring_free(line);
  }
}
