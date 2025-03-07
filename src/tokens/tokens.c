
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Tokens available in eta
**  File: tokens.c
*/

#include <tokens.h>

static const char * const tokenstr[__TOKENCOUNT__] = {
  [TNONE] = "none",
  [TIMPORT] = "import",
  [TSTRUCT] = "struct",
  [TMUT] = "mut",
  [TFOR] = "for",
  [TIF] = "if",
  [TELSE] = "else",
  [TRETURN] = "return",
  [TBREAK] = "break",
  [TCONTINUE] = "continue",
  [TSWITCH] = "switch",
  [TCASE] = "case",
  [TEXTERN] = "extern",
  [TENUM] = "enum",
  [TDEFER] = "defer",
  [TFUNC] = "function",
  [TLET] = "let",
  [TNULL] = "null",

  [TINT] = "int",
  [TFLOAT] = "float",
  [TSTRING] = "string",
  [TBOOL] = "bool",

  [TTVOID] = "void",
  [TTINT] = "int",
  [TTFLOAT] = "float",
  [TTSTRING] = "string",
  [TTBOOL] = "bool",

  [TOPAREN] = "(",
  [TCPAREN] = ")",
  [TOCURLY] = "{",
  [TCCURLY] = "}",
  [TOSQR] = "[",
  [TCSQR] = "]",
  [TCOMMA] = ",",
  [TSEMICOLON] = ";",
  [TCOLON] = ":",
  [TGRT] = ">",
  [TGRE] = ">=",
  [TLES] = "<",
  [TLEE] = "<=",
  [TDOT] = ".",
  [TADD] = "+",
  [TSUB] = "-",
  [TDIV] = "/",
  [TMUL] = "*",
  [TEXP] = "^",
  [TOR] = "||",
  [TAND] = "&&",
  [TUOR] = "|",
  [TUAND] = "&",
  [TPER] = "%",
  [TASS] = "=",
  [TEQL] = "==",
  [TNEQL] = "!=",
  [TUNOT] = "~",
  [TNOT] = "!",
  [TVARIADIC] = "...",
  [TRANGE] = "..",

  [TADAS] = "+=",
  [TSBAS] = "-=",
  [TMLAS] = "*=",
  [TDVAS] = "/=",

  [TIDENTIFIER] = "identifier",
  [TUNKNOWN] = "unknown token",
  [TEOF] = "end of file",
  [TERROR] = "error",
};

const char*
tokenname(enum tokenkind t) {
  return tokenstr[t];
}
