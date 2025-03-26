#include <token.h>

namespace token {
#ifdef TOKENS_DEBUG_INFO_ENABLE
  static const char * const tokenstr[__TOKENCOUNT__] = {
    "none",
    "let",
    "if",
    "else",
    "for",
    "return",
    "extern",
    "function",
    "null",

    "int",
    "float",
    "bool",
    "string",

    "void",
    "int",
    "float",
    "string",
    "bool",

    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    ",",
    ";",
    ":",
    ">",
    ">=",
    "<",
    "<=",
    ".",
    "+",
    "-",
    "*",
    "/",
    "=",
    "==",
    "!=",
    "!",
    "...",
    "..",

    "+=",
    "-=",
    "*=",
    "/=",

    "identifier",
    "end of file",
    "error",
  };

  std::string_view
  to_string(Token t) {
    return tokenstr[t];
  }
#endif
};
