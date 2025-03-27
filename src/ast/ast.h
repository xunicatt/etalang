
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Abstract Syntax Tree implementation
**  File: ast.h
*/

#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <token.h>
#include <cstdint>
#include <lexer.h>
#include <variant>
#include <vector>
#include <string>

namespace ast {
// expr node type
enum ExprType {
  IDENTEXPR = 0,
  NULLLIT,
  INTEGERLIT,
  FLOATLIT,
  BOOLLIT,
  STRINGLIT,
  ARRAYLIT,
  STRUCTLIT,

  UNARYEXP,
  BINARYEXP,

  ASSIGNMENTEXP,
  OPASSIGNMENTEXP,

  CALLEXP,
  INDEXEXP,
  MEMBEREXP,
  __EXPRNODECOUNT__,
};

// stmt node type
enum StmtType {
  STRUCTSTMT = 0,
  LETSTMT,
  BLOCKSTMT,
  RETURNSTMT,
  EXPRESSIONSTMT,
  IFSTMT,
  FORSTMT,
  FUNCTIONSTMT,
  EXTERNSTMT,
  __STMTNODECOUNT__,
};

struct Stmt;
struct BlockStmt;
struct Expr;
struct Identifier;
struct Program;

using StmtRef = std::unique_ptr<Stmt>;
using BlockStmtRef = std::unique_ptr<BlockStmt>;
using ExprRef = std::unique_ptr<Expr>;
using IdentifierRef = std::unique_ptr<Identifier>;
using ProgramRef = std::unique_ptr<Program>;

struct Identifier {
  Location location;
  std::string value;
};

//====================================================
// LITERALS
struct NullLit {
  Location location;
};

struct IntegerLit {
  Location location;
  int64_t value;
};

struct FloatLit {
  Location location;
  double value;
};

struct BoolLit {
  Location location;
  bool value;
};

struct StringLit {
  Location location;
  std::string value;
};

struct ArrayLit {
  Location location;
  std::vector<ExprRef> elements;
};

struct StructLit {
  Location location;
  ExprRef name;
  std::vector<IdentifierRef> names;
  std::vector<ExprRef> value;
};

//====================================================
// EXPRESSIONS
struct UnaryExpr {
  Location location;
  token::Token operator_;
  ExprRef right;
};

struct BinaryExpr {
  Location location;
  token::Token operator_;
  ExprRef left;
  ExprRef right;
};

struct AssignmentExpr {
  Location location;
  ExprRef left;
  ExprRef right;
};

struct CallExpr {
  Location location;
  ExprRef function;
  std::vector<ExprRef> arguments;
};

struct IndexExpr {
  Location location;
  ExprRef left;
  ExprRef index;
};

struct OpAssignmentExpr {
  Location location;
  token::Token operator_;
  ExprRef left;
  ExprRef right;
};

struct MemberExpr {
  Location location;
  ExprRef left;
  IdentifierRef field;
};

using ExprChild = std::variant<
  Identifier,
  NullLit,
  IntegerLit,
  FloatLit,
  BoolLit,
  StringLit,
  ArrayLit,
  StructLit,
  UnaryExpr,
  BinaryExpr,
  AssignmentExpr,
  CallExpr,
  IndexExpr,
  OpAssignmentExpr,
  MemberExpr
>;

struct Expr {
  ExprType type;
  ExprChild child;
};

//====================================================
// STATEMENTS
struct StructStmt {
  IdentifierRef name;
  std::vector<IdentifierRef> types;
  std::vector<IdentifierRef> names;
};

struct LetStmt {
  IdentifierRef name;
  ExprRef value;
};

struct BlockStmt {
  std::vector<StmtRef> stmts;
};

struct IfStmt {
  ExprRef condition;
  BlockStmtRef consequence;
  BlockStmtRef alternative;
};

struct ForStmt {
  StmtRef pre;
  ExprRef condition;
  ExprRef post;
  BlockStmtRef body;
};

struct FunctionStmt {
  IdentifierRef name;
  std::vector<IdentifierRef> parameters;
  BlockStmtRef body;
};

struct ExternStmt {
  IdentifierRef libname;
  IdentifierRef funcname;
  std::vector<token::Token> argtypes;
  token::Token rettype;
};

struct ReturnStmt {
  ExprRef value;
};

struct ExprStmt {
  ExprRef expr;
};

using StmtChild = std::variant<
  StructStmt,
  LetStmt,
  ReturnStmt,
  IfStmt,
  ForStmt,
  FunctionStmt,
  ExprStmt,
  ExternStmt
>;

struct Stmt {
  StmtType type;
  StmtChild child;
};

struct Program {
  std::vector<StmtRef> stmts;
};

Location location(const Expr& e);
Location location(const ExprRef& e);

#ifdef AST_DEBUG_INFO_ENABLE
  #ifndef TOKENS_DEBUG_INFO_ENABLE
    #error TOKENS_DEBUG_INFO_ENABLE is required for AST_DEBUG_INFO_ENABLE
  #endif

  std::string to_string(const ProgramRef& p);
#endif
}; // ast namespace
#endif
