#include <cstdio>
#include <gc.h>
#include <object.h>
#include <token.h>
#include <eval.h>
#include <ast.h>
#include <cassert>
#include <parser.h>
#include <lexer.h>
#include <exec.h>
#include <fstream>
#include <print>
#include <sstream>
#include <cassert>

int
exec(const char* filename) {
  std::ifstream file(filename);
  if(!file) {
    std::println(stderr, "eta: failed to open file '{}'", filename);
    return 1;
  }

  std::stringstream ss;
  ss << file.rdbuf();

  const std::string& data = ss.str();
  file.close();

  std::string _filename(filename);
  Lexer lexer(_filename, data);
  parser::Parser parser(lexer);
  ast::ProgramRef prgm = parser.parse();
  const auto& errors = parser.errors();

  if(errors.size() > 0) {
    for(const auto& err: errors) {
      std::println(stderr, "{}", err);
    }

    return 1;
  }

  assert(prgm != nullptr && "this isn't normal... errors should be > 0");

  Eval eval(lexer);
  Scope scope;

  ObjectRef result = eval.evaluate(prgm, scope);
  ObjectType type = result->type;

  if(type == ObjectType::DERR || type == ObjectType::SERR) {
    fflush(stdout);
    std::println(stderr, "{}", result->value());
  }

  gc::purge();
  return type == ObjectType::DERR;
}
