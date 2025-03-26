
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: Tree walking evaluator
**  File: eval.h
*/

#ifndef __EVAL_H__
#define __EVAL_H__

#include <ast.h>
#include <lexer.h>
#include <object.h>
#include <scope.h>
#include <vector>

ObjectRef serr(const std::string&);

class Eval {
public:
  Eval(Lexer&);
  ObjectRef evaluate(const ast::ProgramRef&, Scope&);

private:
  ObjectRef stmt(const ast::StmtRef&, Scope&);
  ObjectRef let_stmt(const ast::LetStmt&, Scope&);
  ObjectRef return_stmt(const ast::ReturnStmt&, Scope&);
  ObjectRef if_stmt(const ast::IfStmt&, Scope&);
  ObjectRef for_stmt(const ast::ForStmt&, Scope&);
  ObjectRef func_stmt(ast::FunctionStmt&, Scope&);
  ObjectRef extern_stmt(const ast::ExternStmt&, Scope&);
  ObjectRef expr_stmt(const ast::ExprStmt&, Scope&);
  ObjectRef block_stmt(const ast::BlockStmtRef&, Scope&);

  ObjectRef expr(const ast::ExprRef&, Scope&);
  ObjectRef unary_expr(const ast::UnaryExpr&, Scope&);
  ObjectRef binary_expr(const ast::BinaryExpr&, Scope&);

  ObjectRef integer_lit(const ast::IntegerLit&);
  ObjectRef float_lit(const ast::FloatLit&);
  ObjectRef bool_lit(const ast::BoolLit&);
  ObjectRef string_lit(const ast::StringLit&);
  ObjectRef array_lit(const ast::ArrayLit&, Scope&);

  ObjectRef ident_expr(const ast::Identifier&, Scope&);
  ObjectRef assignment_ident(const ast::Identifier&, const ast::ExprRef&, Scope&);
  ObjectRef assignment_array(ObjectRef&, const ast::ExprRef&, const ast::ExprRef&, Scope&);
  ObjectRef assignment_string(ObjectRef&, const ast::ExprRef&, const ast::ExprRef&, Scope&);
  ObjectRef assignment_expr(const ast::AssignmentExpr&, Scope&);
  ObjectRef opassignment_expr(ast::OpAssignmentExpr&, Scope&);
  ObjectRef index_expr(const ast::IndexExpr&, Scope&);
  ObjectRef call_expr(const ast::CallExpr&, Scope&);

  ObjectRef func(const Func&, const Location&, const std::vector<ast::ExprRef>&, Scope&);
  ObjectRef bfunc(const BFunc&, const Location&, const std::vector<ast::ExprRef>&, Scope&);
  ObjectRef efunc(const EFunc&, const Location&, const std::vector<ast::ExprRef>&, Scope&);

  bool is_err(const ObjectRef&);
  ObjectRef derr(const Location&, const ObjectRef&);
  ObjectRef derr(const Location&, const std::string&);

  Lexer& lexer;
};

#endif
