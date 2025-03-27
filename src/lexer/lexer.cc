#include <cmath>
#include <cstddef>
#include <cstdio>
#include <format>
#include <print>
#include <string>
#include <token.h>
#include <cctype>
#include <lexer.h>
#include <map>

using token::Token;

static const std::map<std::string_view, Token> keywords = {
  {"let", Token::LET},
  {"if", Token::IF},
  {"else", Token::ELSE},
  {"for", Token::FOR},
  {"return", Token::RETURN},
  {"extern", Token::EXTERN},
  {"func", Token::FUNC},
  {"true", Token::BOOLLIT},
  {"false", Token::BOOLLIT},
  {"void", Token::VOID},
  {"int", Token::INT},
  {"float", Token::FLOAT},
  {"bool", Token::BOOL},
  {"string", Token::STRING},
  {"null", Token::NULL_},
  {"struct", Token::STRUCT},
};

static const std::map<char, Token> specialchars = {
  {'(', Token::LPAREN},
  {')', Token::RPAREN},
  {'{', Token::LCURLY},
  {'}', Token::RCURLY},
  {'[', Token::LSQR},
  {']', Token::RSQR},
  {',', Token::COMMA},
  {';', Token::SEMICOLON},
  {':', Token::COLON},
  {'>', Token::GRT},
  {'<', Token::LES},
  {'.', Token::DOT},
  {'+', Token::ADD},
  {'-', Token::SUB},
  {'/', Token::DIV},
  {'*', Token::MUL},
  {'=', Token::ASS},
  {'!', Token::NOT},
};

#ifdef TOKENS_DEBUG_INFO_ENABLE
  static void debug(const Token, const Value&);
#endif

Lexer::Lexer(const std::string& filename, const std::string& data):
  _filename(filename), data(data) {
  _lasttoken = Token::NONE;
  loc = {0};
  lastloc = {0};
  _value = {0};
}

bool
Lexer::at_end() const {
  return loc.cursor >= data.length();
}

char
Lexer::current_char() const {
  if(at_end()) return 0;
  return data[loc.cursor];
}

char
Lexer::peek_char() const {
  if(loc.cursor + 1 >= data.length()) return 0;
  return data[loc.cursor + 1];
}

void
Lexer::advance_cursor() {
  if(!at_end() &&
    loc.cursor++ &&
    !at_end() &&
    current_char() == '\n'
  ) {
    loc.row++;
    loc.linebeg = loc.cursor + 1;
  }
}

void
Lexer::save_location() {
  lastloc = loc;
}

void
Lexer::skip_whitespaces() {
  while(!at_end() && isspace(current_char())) {
    advance_cursor();
  }
}

void
Lexer::drop_line() {
  while(!at_end() && current_char() != '\n') {
    advance_cursor();
  }
}

Token
Lexer::next_token_() {
  skip_whitespaces();

  while(current_char() == '#') {
    drop_line();
    skip_whitespaces();
  }

  save_location();
  if(at_end()) return Token::EOF_;

  if(isalpha(current_char()) || current_char() == '_') {
    while(!at_end() &&
      (isalpha(current_char()) || isdigit(current_char()) || current_char() == '_')
    ) {
      advance_cursor();
    }

    std::string ident = data.substr(lastloc.cursor, loc.cursor - lastloc.cursor);
    _value = ident;

    if(!keywords.contains(ident)) {
      return Token::IDENTIFIER;
    }

    Token token = keywords.at(ident);
    if(token == Token::BOOLLIT) {
      _value = ident == "true";
    }

    return token;
  }

  if(isdigit(current_char())) {
    bool isfloat = false;

    while(!at_end() &&
      (isdigit(current_char()) || current_char() == '.')
    ) {
      // [TODO]: Needs error handling
      //         for invalid number
      if(!isfloat && current_char() == '.')
        isfloat = true;
      advance_cursor();
    }

    std::string value = data.substr(lastloc.cursor, loc.cursor - lastloc.cursor);
    if(isfloat) {
      _value = std::stod(value);
      return Token::FLOATLIT;
    }

    _value = std::stoll(value);
    return Token::INTLIT;
  }

  if(current_char() == '\'') {
    advance_cursor();
    std::string str;

    while(!at_end() && current_char() != '\'') {
      char c = current_char(), fc = c;

      if(c == '\\') {
        switch(peek_char()) {
          case 'n':
            fc = '\n';
            break;

          case 't':
            fc = '\t';
            break;

          case '\'':
            fc = '\'';
            break;

          case '\\':
            fc = '\\';
            advance_cursor();
        }
      }

      if(fc != c)
        advance_cursor();

      str += fc;
      advance_cursor();
    }

    _value = str;
    advance_cursor();
    return Token::STRINGLIT;
  }

  if(specialchars.contains(current_char())) {
    Token currtoken = specialchars.at(current_char());
    Token finaltoken = currtoken;

    advance_cursor();
    char nc = current_char();

    if(specialchars.contains(nc)) {
      Token nexttoken = specialchars.at(nc);
      switch(currtoken) {
        case Token::ASS:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::EQL;
          }
          break;

        case Token::GRT:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::GRE;
          }
          break;

        case Token::LES:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::LEE;
          }
          break;

        case Token::NOT:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::NEQL;
          }
          break;

        case Token::ADD:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::ADAS;
          }
          break;

        case Token::SUB:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::SBAS;
          }
          break;

        case Token::MUL:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::MLAS;
          }
          break;

        case Token::DIV:
          if (nexttoken == Token::ASS) {
            finaltoken = Token::DVAS;
          }
          break;

        case Token::DOT: {
          if(nexttoken == Token::DOT) {
            if(peek_char() == '.') {
              advance_cursor();
              finaltoken = Token::VARIADIC;
            } else {
              finaltoken = Token::RANGE;
            }
          }
          break;
        }

        default:
          break;
      }

      if(currtoken != finaltoken) {
        advance_cursor();
      }
    }

    return finaltoken;
  }

  return Token::ERROR;
}

void
Lexer::set_location(const Location& _loc) {
  loc = _loc;
}

Token
Lexer::next_token() {
  _lasttoken = next_token_();
  #ifdef TOKENS_DEBUG_INFO_ENABLE
    debug(_lasttoken, _value);
  #endif
  return _lasttoken;
}

Token
Lexer::last_token() const {
  return _lasttoken;
}

Token
Lexer::peek_token() const {
  Lexer l = *this;
  return l.next_token_();
}

const Value&
Lexer::get_value() const {
  return _value;
}

const Location&
Lexer::location() const {
  return loc;
}

const Location&
Lexer::last_location() const {
  return lastloc;
}

const std::string&
Lexer::filename() const {
  return _filename;
}

std::string
Lexer::current_line() const {
  size_t i = data.find('\n', lastloc.linebeg);
  if(i == std::string::npos) {
    return data;
  }

  return data.substr(lastloc.linebeg, i - lastloc.linebeg);
}

std::string
Lexer::fmt_error(const Location& _loc, const std::string& msg) {
  set_location(_loc);
  next_token();

  std::string errmsg = std::format(
    "\neta: \e[31merror in file: {}:{}:{}\e[0m\n",
    _filename,
    lastloc.row + 1,
    lastloc.cursor - lastloc.linebeg + 1
  );

  errmsg += std::format(
    "{} | {}\n",
    lastloc.row + 1,
    current_line()
  );

  size_t uppyarrw = loc.cursor - lastloc.cursor;
  errmsg += std::format(
    "{}{}   \e[31m{}\e[0m\n",
    std::string(floor(log10(lastloc.row + 1) + 1), ' '),
    std::string(lastloc.cursor - lastloc.linebeg, ' '),
    std::string(uppyarrw + 1 * (uppyarrw <= 0), '^')
  );

  errmsg += std::format(
    "{}{}   \e[31m{}\e[0m",
    std::string(floor(log10(lastloc.row + 1) + 1), ' '),
    std::string(lastloc.cursor - lastloc.linebeg, ' '),
    msg
  );

  return errmsg;
}

#ifdef TOKENS_DEBUG_INFO_ENABLE
  static void
  debug(const Token t, const Value& v) {
    std::print("lexer: token_type: {}", to_string(t));
    switch(t) {
      case Token::INTLIT:
        std::println(" | value: {}", std::get<int64_t>(v));
        break;

      case Token::FLOATLIT:
        std::println(" | value: {}", std::get<double>(v));
        break;

      case Token::BOOLLIT:
        std::println(" | value: {}", std::get<bool>(v) ? "true" : "false");
        break;

      case Token::IDENTIFIER:
      case Token::STRINGLIT:
        std::println(" | value: {}", std::get<std::string>(v));
        break;

      default:
        std::print("\n");
    }
  }
#endif
