
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Lexer to generate tokens from source code
**  File: lexer.h
*/

#ifndef __LEXER_H__
#define __LEXER_H__

#include <cstddef>
#include <cstdint>
#include <token.h>
#include <string>
#include <variant>

struct Location {
  size_t cursor;
  size_t row;
  size_t linebeg;
};

using Value = std::variant<int64_t, double, bool, std::string>;

class Lexer {
public:
  Lexer(const std::string& filename, const std::string& data);
  void set_location(const Location&);
  token::Token next_token();
  token::Token last_token() const;
  token::Token peek_token() const;
  const Value& get_value() const;
  std::string current_line() const;
  const Location& location() const;
  const Location& last_location() const;
  const std::string& filename() const;
  std::string fmt_error(const Location&, const std::string&);

private:
  bool at_end() const;
  char current_char() const;
  char peek_char() const;
  void advance_cursor();
  void save_location();
  void skip_whitespaces();
  void drop_line();
  token::Token next_token_();

  const std::string& _filename;
  const std::string& data;
  token::Token _lasttoken;
  Location loc;
  Location lastloc;
  Value _value;
};

#endif
