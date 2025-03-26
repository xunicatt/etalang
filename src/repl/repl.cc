#include <repl.h>
#include <eval.h>
#include <object.h>
#include <string_view>
#include <token.h>
#include <print>
#include <string>
#include <iostream>
#include <lexer.h>
#include <ast.h>
#include <parser.h>
#include <gc.h>
#include <scope.h>

constexpr std::string_view HELPER = "etalang repl -- type '.help' for help";
constexpr std::string NAME = "repl";
constexpr std::string_view PROMPT = ">> ";
constexpr std::string_view VERSION_STR = "v0.0.2";

#ifdef GC_DEBUG_INFO_ENABLE
  #define GC_MSG ".gc    --- garbage collector\n"
#else
  #define GC_MSG
#endif
constexpr std::string_view HELP_MSG = ".help  --- help\n" \
                                      ".clear --- clear the terminal\n" \
                                      ".ver   --- shows the eta version\n" \
                                      GC_MSG \
                                      ".exit  --- exits the repl\n" ;

void
repl() {
  std::println("\e[32m{} {}\e[0m", HELPER, VERSION_STR);
  std::string line;
  Scope scope;

  while(true) {
    std::print(PROMPT);
    std::getline(std::cin, line);

    if(line.length() > 0 && line[0] == '.') {
      if(line == ".help") {
        std::println("\e[32m{}\e[0m", HELP_MSG);
        continue;
      }

      if(line == ".clear") {
        std::println("\033c");
        continue;
      }

      if(line == ".ver") {
        std::println(VERSION_STR);
        continue;
      }

      #ifdef GC_DEBUG_INFO_ENABLE
        if(line == ".gc") {
          std::println("========= GC ==========");
          std::println("before cleanup:\n\tobjects: {}", gc::count());
          gc::collect();
          std::println("-----------------------");
          std::println("after cleanup:\n\tobjects: {}", gc::count());
          std::println("=======================");
          continue;
        }
      #endif

      if(line == ".exit") {
        gc::purge();
        return;
      }

      std::println("\e[31munknown command: {}\e[0m", line);
      continue;
    }

    Lexer lexer(NAME, line) ;
    parser::Parser parser(lexer);

    ast::ProgramRef prgm = parser.parse();
    if(parser.errors().size() > 0) {
      for(const auto &e: parser.errors()) {
        std::println(stderr, "{}", e);
      }
      continue;
    }

    #ifdef AST_DEBUG_INFO_ENABLE
      std::println("{}", to_string(prgm));
    #endif

    Eval eval(lexer);
    ObjectRef result = eval.evaluate(std::move(prgm), scope);
    std::println("{}", result->value());
  }
}
