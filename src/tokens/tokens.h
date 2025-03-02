
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Tokens available in eta
**  File: tokens.h
*/

#ifndef __TOKENS_H__
#define __TOKENS_H__

enum tokenkind {
  TNONE = 0,
  TIMPORT,
  TSTRUCT,
  TMUT,
  TFOR,
  TIF,
  TELSE,
  TRETURN,
  TBREAK,
  TCONTINUE,
  TSWITCH,
  TCASE,
  TEXTERN,
  TENUM,
  TDEFER,
  TFUNC,
  TLET,
  TNULL,

  TINT,
  TFLOAT,
  TSTRING,
  TBOOL,

  TOPAREN,
  TCPAREN,
  TOCURLY,
  TCCURLY,
  TOSQR,
  TCSQR,
  TCOMMA,
  TSEMICOLON,
  TCOLON,
  TGRT,
  TGRE,
  TLES,
  TLEE,
  TDOT,
  TADD,
  TSUB,
  TDIV,
  TMUL,
  TEXP,
  TOR,
  TAND,
  TUOR,
  TUAND,
  TPER,
  TASS,
  TEQL,
  TNEQL,
  TUNOT,
  TNOT,

  TADAS,
  TSBAS,
  TMLAS,
  TDVAS,

  TIDENTIFIER,
  TUNKNOWN,
  TEOF,
  TERROR,
  __TOKENCOUNT__,
};

const char* tokenname(enum tokenkind);

#endif
