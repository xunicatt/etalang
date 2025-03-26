
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: An implementation of Pratt Parser
**  File: parser.h
*/

#ifndef __PARSER_H__
#define __PARSER_H__

#include <optional>
#include <string>
#include <token.h>
#include <lexer.h>
#include <ast.h>
#include <vector>

namespace parser {
enum Precedence {
  NONE = 0,
  LOWEST,
  ASSIGNMENT,
  EQUALS,
  LESSGREATER,
  SUM,
  PRODUCT,
  PREFIX,
  CALL,
  INDEX,
};

class Parser {
public:
  Parser(Lexer&);
  const std::vector<std::string>& errors();
  ast::ProgramRef parse();

private:
  inline void error(const std::string&);
  inline void error(const std::string&, const Location&);
  ast::StmtRef stmt();
  ast::StmtRef let_stmt();
  ast::StmtRef return_stmt();
  ast::StmtRef if_stmt();
  ast::StmtRef for_stmt();
  ast::StmtRef func_stmt();
  ast::StmtRef extern_stmt();
  ast::StmtRef expr_stmt();
  ast::BlockStmtRef block_stmt();

  ast::ExprRef expr(Precedence);
  ast::ExprRef unary_expr();
  ast::ExprRef binary_expr(ast::ExprRef);
  ast::ExprRef grouped_expr();
  std::optional<std::vector<ast::ExprRef>>
  list_expr(const token::Token&);

  ast::ExprRef null_lit();
  ast::ExprRef integer_lit();
  ast::ExprRef float_lit();
  ast::ExprRef bool_lit();
  ast::ExprRef string_lit();
  ast::ExprRef array_lit();

  ast::ExprRef ident_expr();
  ast::ExprRef assignment_expr(ast::ExprRef);
  ast::ExprRef opassignment_expr(ast::ExprRef);
  ast::ExprRef index_expr(ast::ExprRef);
  ast::ExprRef call_expr(ast::ExprRef);

  Precedence peek_precedence();
  Precedence current_precedence();

  Lexer &lexer;
  std::vector<std::string> _errors;
};
};

#endif
