
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: eta is a tree walking interpreted language
**                built for fun
**  File: main.c
**
**  Interpreter Version: v0.0.1
*/

#include <stdint.h>
#include <repl.h>
#include <exec.h>

int32_t
main(int32_t argc, char *argv[]) {
  if(argc < 2) {
    repl();
    return 0;
  }

  return exec(argv[1]);
}
