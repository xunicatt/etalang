#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <parser.h>
#include <string>
#include <token.h>
#include <lexer.h>
#include <ast.h>
#include <utility>
#include <vector>
#include <map>
#include <print>

#define TO_UNARY_FN(fn) [this](void) { return this->fn(); }
#define TO_BINARY_FN(fn) [this](ast::ExprRef left) { return this->fn(std::move(left)); }

using token::Token;
using ast::ExprType;
using ast::StmtType;

using UnaryParseFn = std::function<ast::ExprRef(void)>;
using BinaryParseFn = std::function<ast::ExprRef(ast::ExprRef)>;

namespace parser {
static std::map<Token, UnaryParseFn> unaryfns;
static std::map<Token, BinaryParseFn> binaryfns;

static const std::map<Token, parser::Precedence> precedences = {
  {Token::ASS, Precedence::ASSIGNMENT},
  {Token::ADAS, Precedence::ASSIGNMENT},
  {Token::SBAS, Precedence::ASSIGNMENT},
  {Token::MLAS, Precedence::ASSIGNMENT},
  {Token::DVAS, Precedence::ASSIGNMENT},
  {Token::EQL, Precedence::EQUALS},
  {Token::NEQL, Precedence::EQUALS},
  {Token::LES, Precedence::LESSGREATER},
  {Token::LEE, Precedence::LESSGREATER},
  {Token::GRT, Precedence::LESSGREATER},
  {Token::GRE, Precedence::LESSGREATER},
  {Token::ADD, Precedence::SUM},
  {Token::SUB, Precedence::SUM},
  {Token::DIV, Precedence::PRODUCT},
  {Token::MUL, Precedence::PRODUCT},
  {Token::LPAREN, Precedence::CALL},
  {Token::LSQR, Precedence::INDEX},
};

static const std::map<Token, Token> opassignment = {
  {Token::ADAS, Token::ADD},
  {Token::SBAS, Token::SUB},
  {Token::MLAS, Token::MUL},
  {Token::DVAS, Token::DIV}
};

static inline bool
is_a_type(Token t) {
  return t == Token::INT || t == Token::FLOAT  ||
          t == Token::BOOL || t == Token::STRING;
}

static inline bool
is_semicolon_req(const ast::StmtRef& s) {
  if(s == nullptr) return false;
  StmtType type = s->type;
  return type != StmtType::IFSTMT && type != StmtType::FORSTMT && type != StmtType::FUNCTIONSTMT;
}

Parser::Parser(Lexer& lexer)
  : lexer(lexer) {
  unaryfns[Token::IDENTIFIER] = TO_UNARY_FN(ident_expr);
  unaryfns[Token::NULL_] = TO_UNARY_FN(null_lit);
  unaryfns[Token::INTLIT] = TO_UNARY_FN(integer_lit);
  unaryfns[Token::FLOATLIT] = TO_UNARY_FN(float_lit);
  unaryfns[Token::BOOLLIT] = TO_UNARY_FN(bool_lit);
  unaryfns[Token::STRINGLIT] = TO_UNARY_FN(string_lit);
  unaryfns[Token::LSQR] = TO_UNARY_FN(array_lit);

  auto unary_expr = TO_UNARY_FN(unary_expr);
  unaryfns[Token::NOT] = unary_expr;
  unaryfns[Token::SUB] = unary_expr;
  unaryfns[Token::LPAREN] = TO_UNARY_FN(grouped_expr);

  auto binary_expr = TO_BINARY_FN(binary_expr);
  binaryfns[Token::ADD] = binary_expr;
  binaryfns[Token::SUB] = binary_expr;
  binaryfns[Token::MUL] = binary_expr;
  binaryfns[Token::DIV] = binary_expr;
  binaryfns[Token::EQL] = binary_expr;
  binaryfns[Token::NEQL] = binary_expr;
  binaryfns[Token::LES] = binary_expr;
  binaryfns[Token::LEE] = binary_expr;
  binaryfns[Token::GRT] = binary_expr;
  binaryfns[Token::GRE] = binary_expr;

  binaryfns[Token::ASS] = TO_BINARY_FN(assignment_expr);
  binaryfns[Token::ADAS] = TO_BINARY_FN(opassignment_expr);
  binaryfns[Token::SBAS] = TO_BINARY_FN(opassignment_expr);
  binaryfns[Token::MLAS] = TO_BINARY_FN(opassignment_expr);
  binaryfns[Token::DVAS] = TO_BINARY_FN(opassignment_expr);
  binaryfns[Token::LSQR] = TO_BINARY_FN(index_expr);
  binaryfns[Token::LPAREN] = TO_BINARY_FN(call_expr);
}

const std::vector<std::string>&
Parser::errors() {
  return _errors;
}

ast::ProgramRef
Parser::parse() {
  ast::ProgramRef prgm = std::make_unique<ast::Program>();

  while(lexer.next_token() != Token::EOF_) {
    if(lexer.last_token() == Token::ERROR) {
      error("unknown token");
      return nullptr;
    }

    ast::StmtRef _stmt = stmt();
    if(_stmt == nullptr) {
      return nullptr;
    }

    prgm->stmts.push_back(std::move(_stmt));
  }

  return prgm;
}

inline void
Parser::error(const std::string& msg) {
  error(msg, lexer.location());
}

inline void
Parser::error(const std::string& msg, const Location& loc) {
  _errors.push_back(lexer.fmt_error(loc, msg));
}

ast::StmtRef
Parser::stmt() {
  ast::StmtRef stmt;

  switch(lexer.last_token()) {
  case Token::LET:
    stmt = let_stmt();
    break;

  case Token::RETURN:
    stmt = return_stmt();
    break;

  case Token::IF:
    stmt = if_stmt();
    break;

  case Token::FOR:
    stmt = for_stmt();
    break;

  case Token::FUNC:
    stmt = func_stmt();
    break;

  case Token::EXTERN:
    stmt = extern_stmt();
    break;

  default:
    stmt = expr_stmt();
  }

  if(is_semicolon_req(stmt)) {
    if(lexer.peek_token() != Token::SEMICOLON) {
      error("expected ;");
      return nullptr;
    }
    lexer.next_token();
  }

  return stmt;
}

ast::StmtRef
Parser::let_stmt() {
  //let x = <expr>
  if(lexer.peek_token() != Token::IDENTIFIER) {
    error("expected identifier");
    return nullptr;
  }

  lexer.next_token();
  ast::IdentifierRef name = std::make_unique<ast::Identifier>(ast::Identifier{
    lexer.last_location(), std::get<std::string>(lexer.get_value())
  });

  if(lexer.peek_token() != Token::ASS) {
    error("a variable must be initialized with a value");
    return nullptr;
  }

  lexer.next_token();
  lexer.next_token();

  ast::ExprRef value;
  if((value = expr(Precedence::LOWEST)) == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Stmt>(ast::Stmt{
    .type = StmtType::LETSTMT, .child = ast::LetStmt{
      .name = std::move(name),
      .value = std::move(value)
    }
  });
}

ast::StmtRef
Parser::return_stmt() {
  //return or return <expr>
  ast::ExprRef value;
  if(lexer.peek_token() != Token::SEMICOLON) {
    lexer.next_token(); //fetch token
    value = expr(Precedence::LOWEST);
  }

  return std::make_unique<ast::Stmt>(ast::Stmt{
    .type = StmtType::RETURNSTMT,
    .child = ast::ReturnStmt{
      .value = std::move(value)
    }
  });
}

ast::StmtRef
Parser::if_stmt() {
  lexer.next_token();

  ast::ExprRef condition = expr(Precedence::LOWEST);
  if(condition == nullptr) {
    return nullptr;
  }

  if(lexer.peek_token() != Token::LCURLY) {
    error("expected {");
    return nullptr;
  }
  lexer.next_token();

  ast::BlockStmtRef consequence = block_stmt();
  if(consequence == nullptr) {
    return nullptr;
  }

  ast::BlockStmtRef alternative;
  if(lexer.peek_token() == Token::ELSE) {
    lexer.next_token();

    if(lexer.peek_token() != Token::LCURLY) {
      error("expected {");
      return nullptr;
    }
    lexer.next_token();

    if((alternative = block_stmt()) == nullptr) {
      return nullptr;
    }
  }

  return std::make_unique<ast::Stmt>(ast::Stmt{
    .type = StmtType::IFSTMT,
    .child = ast::IfStmt{
      .condition = std::move(condition),
      .consequence = std::move(consequence),
      .alternative = std::move(alternative)
    }
  });
}

ast::StmtRef
Parser::for_stmt() {
  ast::StmtRef pre;
  if(lexer.peek_token() != Token::SEMICOLON) {
    lexer.next_token();
    if((pre = stmt()) == nullptr) {
      return nullptr;
    }
  } else {
    lexer.next_token();
  }


  ast::ExprRef condition;
  if(lexer.peek_token() != Token::SEMICOLON) {
    lexer.next_token();
    if((condition = expr(Precedence::LOWEST)) == nullptr) {
      return nullptr;
    }
  }

  if(lexer.peek_token() != Token::SEMICOLON) {
    error("expected ;");
    return nullptr;
  }
  lexer.next_token();

  ast::ExprRef post;
  if(lexer.peek_token() != Token::LCURLY) {
    lexer.next_token();
    if((post = expr(Precedence::LOWEST)) == nullptr) {
      return nullptr;
    }
  }

  if(lexer.peek_token() != Token::LCURLY) {
    error("expected {");
    return nullptr;
  }
  lexer.next_token();

  ast::BlockStmtRef body = block_stmt();
  if(body == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Stmt>(ast::Stmt{
    .type = StmtType::FORSTMT,
    .child = ast::ForStmt{
      .pre = std::move(pre),
      .condition = std::move(condition),
      .post = std::move(post),
      .body = std::move(body)
    }
  });
}

ast::StmtRef
Parser::func_stmt() {
  if(lexer.peek_token() != Token::IDENTIFIER) {
    error("expected identifier");
    return nullptr;
  }
  lexer.next_token();

  ast::IdentifierRef name = std::make_unique<ast::Identifier>(ast::Identifier{
    .location = lexer.last_location(),
    .value = std::get<std::string>(lexer.get_value())
  });

  if(lexer.peek_token() != Token::LPAREN) {
    error("expected (");
    return nullptr;
  }
  lexer.next_token();

  std::vector<ast::IdentifierRef> paramenters;

  if(lexer.peek_token() != Token::RPAREN) {
    while(true) {
      if(lexer.peek_token() != Token::IDENTIFIER) {
        error("expected an identifier");
        return nullptr;
      }

      lexer.next_token();
      paramenters.push_back(
        std::make_unique<ast::Identifier>(ast::Identifier{
          .location = lexer.last_location(),
          .value = std::get<std::string>(lexer.get_value())
        })
      );

      if(lexer.peek_token() == Token::RPAREN) {
        break;
      }

      if(lexer.peek_token() != Token::COMMA) {
        error("expected , or )");
        return nullptr;
      }

      lexer.next_token();
    }
  }

  if(lexer.peek_token() != Token::RPAREN) {
    error("expected )");
    return nullptr;
  }
  lexer.next_token();

  if(lexer.peek_token() != Token::LCURLY) {
    error("expected {");
    return nullptr;
  }
  lexer.next_token();

  ast::BlockStmtRef body = block_stmt();
  if(body == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Stmt>(ast::Stmt{
    .type = StmtType::FUNCTIONSTMT,
    .child = ast::FunctionStmt{
      .name = std::move(name),
      .parameters = std::move(paramenters),
      .body = std::move(body)
    }
  });
}

ast::StmtRef
Parser::extern_stmt() {
  //extern <libname> func <funcname> (<args-types>): <return-type>
  if(lexer.peek_token() != Token::IDENTIFIER) {
    error("expected library name");
    return nullptr;
  }

  lexer.next_token();
  ast::IdentifierRef libname = std::make_unique<ast::Identifier>(ast::Identifier{
    lexer.last_location(),
    std::get<std::string>(lexer.get_value())
  });

  if(lexer.peek_token() != Token::FUNC) {
    error("expected 'func'");
    return nullptr;
  }

  lexer.next_token();
  if(lexer.peek_token() != Token::IDENTIFIER) {
    error("expected function name");
    return nullptr;
  }

  lexer.next_token();
  ast::IdentifierRef funcname = std::make_unique<ast::Identifier>(ast::Identifier {
    lexer.last_location(),
    std::get<std::string>(lexer.get_value())
  });

  if(lexer.peek_token() != Token::LPAREN) {
    error("expected (");
    return nullptr;
  }

  std::vector<Token> argtypes;

  lexer.next_token();
  if(lexer.peek_token() != Token::RPAREN) {
    while(true) {
      Token argtype = lexer.peek_token();
      if(argtype == Token::VARIADIC) {
        lexer.next_token();
        argtypes.push_back(argtype);

        if(lexer.peek_token() != Token::RPAREN) {
          error("expected )");
          return nullptr;
        }

        break;
      }

      if(!is_a_type(argtype)) {
        error("expected a type");
        return nullptr;
      }

      lexer.next_token();
      argtypes.push_back(argtype);

      if(lexer.peek_token() == Token::RPAREN) {
        break;
      }

      if(lexer.peek_token() != Token::COMMA) {
        error("expected , or )");
        return nullptr;
      }

      lexer.next_token();
    }
  }

  lexer.next_token();

  if(lexer.peek_token() != Token::COLON) {
    error("expected :");
    return nullptr;
  }

  lexer.next_token();
  if(!is_a_type(lexer.peek_token()) && lexer.peek_token() != Token::VOID) {
    error("expected a return type");
    return nullptr;
  }

  Token rettype = lexer.next_token();

  return std::make_unique<ast::Stmt>(ast::Stmt{
    .type = StmtType::EXTERNSTMT,
    .child = ast::ExternStmt{
      .libname = std::move(libname),
      .funcname = std::move(funcname),
      .argtypes = argtypes,
      .rettype = rettype
    }
  });
}

ast::StmtRef
Parser::expr_stmt() {
  ast::ExprRef _expr = expr(Precedence::LOWEST);
  if(_expr == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Stmt>(ast::Stmt{
    .type = StmtType::EXPRESSIONSTMT,
    .child = ast::ExprStmt{
      .expr = std::move(_expr)
    }
  });
}

ast::BlockStmtRef
Parser::block_stmt() {
  lexer.next_token();
  std::vector<ast::StmtRef> stmts;

  while(true) {
    if(lexer.last_token() == Token::EOF_) {
      error("expected }");
      return nullptr;
    }

    if(lexer.last_token() == Token::RCURLY) {
      break;
    }

    ast::StmtRef _stmt = stmt();
    if(_stmt == nullptr) {
      return nullptr;
    }

    stmts.push_back(std::move(_stmt));
    lexer.next_token();
  }

  return std::make_unique<ast::BlockStmt>(ast::BlockStmt{
    .stmts = std::move(stmts)
  });
}


ast::ExprRef
Parser::expr(Precedence p) {
  if(!unaryfns.contains(lexer.last_token())) {
    error("unknown unary operator", lexer.last_location());
    return nullptr;
  }

  UnaryParseFn ufn = unaryfns[lexer.last_token()];
  ast::ExprRef left = ufn();
  if(left == nullptr) {
    return nullptr;
  }

  while(lexer.peek_token() != Token::SEMICOLON && p < peek_precedence()) {
    if(!binaryfns.contains(lexer.peek_token())) {
      return left;
    }

    BinaryParseFn bfn = binaryfns[lexer.next_token()];
    if((left = bfn(std::move(left))) == nullptr) {
      return nullptr;
    }
  }

  return left;
}

ast::ExprRef
Parser::unary_expr() {
  const Location loc = lexer.last_location();
  const Token& operator_ = lexer.last_token();

  lexer.next_token();
  ast::ExprRef right = expr(Precedence::PREFIX);
  if(right == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ExprType::UNARYEXP,
    .child = ast::UnaryExpr{
      .location = loc,
      .operator_ = operator_,
      .right = std::move(right)
    }
  });
}

ast::ExprRef
Parser::binary_expr(ast::ExprRef left) {
  const Location loc = lexer.last_location();
  const Token& operator_ = lexer.last_token();
  const Precedence& prec = current_precedence();

  lexer.next_token();
  ast::ExprRef right = expr(prec);
  if(right == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ExprType::BINARYEXP,
    .child = ast::BinaryExpr{
      .location = loc,
      .operator_ = operator_,
      .left = std::move(left),
      .right = std::move(right)
    }
  });
}

ast::ExprRef
Parser::grouped_expr() {
  lexer.next_token();
  ast::ExprRef _expr = expr(Precedence::LOWEST);
  if(_expr == nullptr) {
    return nullptr;
  }

  if(lexer.peek_token() != Token::RPAREN) {
    error("expected )");
    return nullptr;
  }

  lexer.next_token();
  return _expr;
}

std::optional<std::vector<ast::ExprRef>>
Parser::list_expr(const token::Token& end) {
  std::vector<ast::ExprRef> exprs;
  if(lexer.next_token() == end) {
    return exprs;
  }

  while(true) {
    ast::ExprRef _expr = expr(Precedence::LOWEST);
    if(_expr == nullptr) {
      return std::nullopt;
    }

    exprs.push_back(std::move(_expr));
    if(lexer.next_token() == end) {
      break;
    }

    if(lexer.last_token() != Token::COMMA) {
      error("expected , or )");
      return std::nullopt;
    }

    lexer.next_token();
  }

  return exprs;
}

ast::ExprRef
Parser::null_lit() {
  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ast::NULLLIT,
    .child = ast::NullLit{
      .location = lexer.last_location()
    }
  });
}

ast::ExprRef
Parser::integer_lit() {
  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ast::INTEGERLIT,
    .child = ast::IntegerLit{
      .location = lexer.last_location(),
      .value = std::get<int64_t>(lexer.get_value())
    }
  });
}

ast::ExprRef
Parser::float_lit() {
  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ast::FLOATLIT,
    .child = ast::FloatLit{
      .location = lexer.last_location(),
      .value = std::get<double>(lexer.get_value())
    }
  });
}

ast::ExprRef
Parser::bool_lit() {
  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ast::BOOLLIT,
    .child = ast::BoolLit{
      .location = lexer.last_location(),
      .value = std::get<bool>(lexer.get_value())
    }
  });
}

ast::ExprRef
Parser::string_lit() {
  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ast::STRINGLIT,
    .child = ast::StringLit{
      .location = lexer.last_location(),
      .value = std::get<std::string>(lexer.get_value())
    }
  });
}

ast::ExprRef
Parser::array_lit() {
  const Location loc = lexer.last_location();
  auto elements = list_expr(Token::RSQR);
  if(!elements.has_value()) {
    return nullptr;
  }

  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ExprType::ARRAYLIT,
    .child = ast::ArrayLit{
      .location = loc,
      .elements = std::move(*elements)
    }
  });
}

ast::ExprRef
Parser::ident_expr() {
  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ExprType::IDENTEXPR,
    .child = ast::Identifier{
      .location = lexer.last_location(),
      .value = std::get<std::string>(lexer.get_value())
    }
  });
}

ast::ExprRef
Parser::assignment_expr(ast::ExprRef left) {
  const Location loc = lexer.last_location();
  lexer.next_token();

  ast::ExprRef right = expr(Precedence::LOWEST);
  if(right == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ExprType::ASSIGNMENTEXP,
    .child = ast::AssignmentExpr{
      .location = loc,
      .left = std::move(left),
      .right = std::move(right)
    }
  });
}

ast::ExprRef
Parser::opassignment_expr(ast::ExprRef left) {
  const Location loc = lexer.last_location();
  Token operator_ = opassignment.at(lexer.last_token());

  lexer.next_token();
  ast::ExprRef right = expr(Precedence::LOWEST);
  if(right == nullptr) {
    return nullptr;
  }

  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ast::OPASSIGNMENTEXP,
    .child = ast::OpAssignmentExpr{
      .location = loc,
      .operator_ = operator_,
      .left = std::move(left),
      .right = std::move(right)
    }
  });
}

ast::ExprRef
Parser::index_expr(ast::ExprRef left) {
  const Location loc = lexer.location();

  lexer.next_token();
  ast::ExprRef index = expr(Precedence::LOWEST);
  if(index == nullptr) {
    return nullptr;
  }

  if(lexer.peek_token() != Token::RSQR) {
    error("expected ]");
    return nullptr;
  }
  lexer.next_token();

  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ExprType::INDEXEXP,
    .child = ast::IndexExpr{
      .location = loc,
      .left = std::move(left),
      .index = std::move(index)
    }
  });
}

ast::ExprRef
Parser::call_expr(ast::ExprRef left) {
  auto arguments = list_expr(Token::RPAREN);
  if(!arguments.has_value()) {
    return nullptr;
  }

  return std::make_unique<ast::Expr>(ast::Expr{
    .type = ast::CALLEXP,
    .child = ast::CallExpr{
      .function = std::move(left),
      .arguments = std::move(*arguments)
    }
  });
}

Precedence
Parser::peek_precedence() {
  const Token& t = lexer.peek_token();
  return precedences.contains(t) ? precedences.at(t) : Precedence::LOWEST;
}

Precedence
Parser::current_precedence() {
  const Token& t = lexer.last_token();
  return precedences.contains(t) ? precedences.at(t) : Precedence::LOWEST;
}
};
