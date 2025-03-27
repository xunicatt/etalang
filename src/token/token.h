
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Tokens available in eta
**  File: tokens.h
*/

#ifndef __TOKENS_H__
#define __TOKENS_H__

#ifdef TOKENS_DEBUG_INFO_ENABLE
  #include <string_view>
#endif

namespace token {
enum Token {
  NONE = 0,
  LET,
  IF,
  ELSE,
  FOR,
  RETURN,
  EXTERN,
  FUNC,
  NULL_,
  STRUCT,

  // values
  INTLIT,
  FLOATLIT,
  BOOLLIT,
  STRINGLIT,

  // types
  VOID,
  INT,
  FLOAT,
  STRING,
  BOOL,

  LPAREN,
  RPAREN,
  LCURLY,
  RCURLY,
  LSQR,
  RSQR,
  COMMA,
  SEMICOLON,
  COLON,
  GRT,
  GRE,
  LES,
  LEE,
  DOT,
  ADD,
  SUB,
  MUL,
  DIV,
  ASS,
  EQL,
  NEQL,
  NOT,
  VARIADIC,
  RANGE,

  ADAS,
  SBAS,
  MLAS,
  DVAS,

  IDENTIFIER,
  EOF_,
  ERROR,
  __TOKENCOUNT__,
};

#if TOKENS_DEBUG_INFO_ENABLE
  std::string_view to_string(Token);
#endif
};

#endif
