#include <print>
#include <repl.h>
#include <exec.h>
#include <string>

int
main(int argc, char* argv[]) {
  if(argc < 2) {
    repl();
    return 0;
  }

  if(argv[1] == std::string("--help")) {
    std::println("usage: eta <filename>.n");
    return 0;
  }

  if(argv[1] == std::string("--version")) {
    std::println("v0.0.1");
    return 0;
  }

  return exec(argv[1]);
}
