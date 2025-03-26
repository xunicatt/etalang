#include <cstddef>
#include <format>
#include <string>
#include <token.h>
#include <lexer.h>
#include <ast.h>
#include <variant>

namespace ast {
#ifdef AST_DEBUG_INFO_ENABLE
  static const char* exprstr[__EXPRNODECOUNT__] = {
    "ident",
    "null",
    "int",
    "float",
    "bool",
    "string",
    "array",

    "unary",
    "binary",

    "assignment",
    "op-assignment",

    "call",
    "index",
  };

  static const char* stmtstr[__STMTNODECOUNT__] = {
    "let",
    "block",
    "return",
    "expr",
    "if",
    "for",
    "function",
    "extern",
  };

  static inline std::string to_string(const IdentifierRef& ir);
  static std::string to_string(const BlockStmtRef& bs);
  static inline std::string to_string(const ExprRef& er);
  static inline std::string to_string(const StmtRef& sr);

  static inline std::string_view
  to_string(ExprType t) {
    return exprstr[t];
  }

  static inline std::string_view
  to_string(StmtType t) {
    return stmtstr[t];
  }

  static inline std::string
  to_string(const Identifier& ir) {
    return std::format("{{{}: {}}}", to_string(IDENTEXPR), ir.value);
  }

  static inline std::string
  to_string(const IdentifierRef& ir) {
    return to_string(*ir);
  }

  static inline std::string
  to_string(const NullLit& nl) {
    return std::format("{}", to_string(NULLLIT));
  }

  static inline std::string
  to_string(const IntegerLit& il) {
    return std::format("{{{}: {}}}", to_string(INTEGERLIT), il.value);
  }

  static inline std::string
  to_string(const FloatLit& fl) {
    return std::format("{{{}: {}}}", to_string(FLOATLIT), fl.value);
  }

  static inline std::string
  to_string(const BoolLit& bl) {
    return std::format("{{{}: {}}}", to_string(BOOLLIT), bl.value ? "true" : "false");
  }

  static inline std::string
  to_string(const StringLit& sl) {
    return std::format("{{{}: {}}}", to_string(STRINGLIT), sl.value);
  }

  static std::string
  to_string(const ArrayLit& al) {
    std::string str = std::format("{{{}: [", to_string(ARRAYLIT));
    for(size_t i = 0; i < al.elements.size(); i++) {
      str += to_string(al.elements[i]);
      str += i < al.elements.size() - 1 ? ", " : "";
    }
    str += "]}";
    return str;
  }

  static inline std::string
  to_string(const UnaryExpr& ue) {
    return std::format(
      "{{{}: {{op: {}, right: {}}}}}",
      to_string(UNARYEXP),
      to_string(ue.operator_),
      to_string(ue.right)
    );
  }

  static inline std::string
  to_string(const BinaryExpr& be) {
    return std::format(
      "{{{}: {{op: {}, left: {}, right: {}}}}}",
      to_string(BINARYEXP),
      to_string(be.operator_),
      to_string(be.left),
      to_string(be.right)
    );
  }

  static inline std::string
  to_string(const AssignmentExpr& ae) {
    return std::format(
      "{{{}: {{left: {},right: {}}}}}",
      to_string(ASSIGNMENTEXP),
      to_string(ae.left),
      to_string(ae.right)
    );
  }

  static std::string
  to_string(const CallExpr& ce) {
    std::string args = "[";
    for(size_t i = 0; i < ce.arguments.size(); i++) {
      args += to_string(ce.arguments[i]);
      args += i < ce.arguments.size() - 1 ? ", " : "";
    }
    args += "]";

    return std::format(
      "{{{}: {{func: {}, args: {}}}}}",
      to_string(CALLEXP),
      to_string(ce.function),
      args
    );
  }

  static inline std::string
  to_string(const IndexExpr& ie) {
    return std::format(
      "{{{}: {{left: {}, index: {}}}}}",
      to_string(INDEXEXP),
      to_string(ie.left),
      to_string(ie.index)
    );
  }

  static inline std::string
  to_string(const OpAssignmentExpr& oe) {
    return std::format(
      "{{{}: {{op: {}, left: {}, right: {}}}}}",
      to_string(OPASSIGNMENTEXP),
      to_string(oe.operator_),
      to_string(oe.left),
      to_string(oe.right)
    );
  }

  static inline std::string
  to_string(const ExprRef& er) {
    return std::visit([](const auto& child) { return to_string(child);}, er->child);
  }

  static inline std::string
  to_string(const LetStmt& ls) {
    return std::format(
      "{{{}: {{name: {}, value: {}}}}}",
      to_string(LETSTMT),
      to_string(ls.name),
      to_string(ls.value)
    );
  }

  static std::string
  to_string(const BlockStmtRef& bs) {
    std::string res = "[";
    for(size_t i = 0; i < bs->stmts.size(); i++) {
      res += to_string(bs->stmts[i]);
      res += i < bs->stmts.size() - 1 ? ", " : "";
    }
    res += "]";

    return std::format(
      "{{{}: {}}}",
      to_string(BLOCKSTMT),
      res
    );
  }

  static inline std::string
  to_string(const IfStmt& ie) {
    return std::format(
      "{{{}: {{cond: {}, true: {}, false: {}}}}}",
      to_string(IFSTMT),
      to_string(ie.condition),
      to_string(ie.consequence),
      (ie.alternative) ? to_string(ie.alternative) : "nil"
    );
  }

  static inline std::string
  to_string(const ForStmt& fe) {
    return std::format(
      "{{{}: {{init: {}, cond: {}, updt: {}, body: {}}}}}",
      to_string(FORSTMT),
      fe.pre != nullptr ? to_string(fe.pre) : "nil",
      fe.condition != nullptr ? to_string(fe.condition) : "nil",
      fe.post != nullptr ? to_string(fe.post) : "nil",
      to_string(fe.body)
    );
  }

  static std::string
  to_string(const ExternStmt& es) {
    std::string argstypes = "[";
    for(size_t i = 0; i < es.argtypes.size(); i++) {
      argstypes += to_string(es.argtypes[i]);
      argstypes += i < es.argtypes.size() - 1 ? ", " : "";
    }
    argstypes += "]";

    return std::format(
      "{{{}: {{lib: {}, func: {}, args-types: {}, ret-type: {}}}}}",
      to_string(EXTERNSTMT),
      to_string(es.libname),
      to_string(es.funcname),
      argstypes,
      to_string(es.rettype)
    );
  }

  static inline std::string
  to_string(const ReturnStmt& rs) {
    return std::format(
      "{{{}: {}}}",
      to_string(RETURNSTMT),
      rs.value != nullptr ? to_string(rs.value) : "nil"
    );
  }

  static std::string
  to_string(const FunctionStmt& fl) {
    std::string param = "[";
    for(size_t i = 0; i < fl.parameters.size(); i++) {
      param += to_string(fl.parameters[i]);
      param += i < fl.parameters.size() - 1 ? ", " : "";
    }
    param += "]";

    return std::format(
      "{{{}: {{name: {}, param: {}, body: {}}}}}",
      to_string(FUNCTIONSTMT),
      to_string(fl.name),
      param,
      to_string(fl.body)
    );
  }


  static inline std::string
  to_string(const ExprStmt& es) {
    return to_string(es.expr);
  }

  static inline std::string
  to_string(const StmtRef& sr) {
    return std::visit([](const auto& child) { return to_string(child);}, sr->child);
  }

  std::string
  to_string(const ProgramRef& p) {
    std::string stmtstr = "[";
    for(size_t i = 0; i < p->stmts.size(); i++) {
      stmtstr += to_string(p->stmts[i]);
      stmtstr += i < p->stmts.size() - 1 ? ", " : "";
    }
    stmtstr += "]";

    return std::format(
      "{{program: {}}}",
      stmtstr
    );
  }
#endif


Location location(const Expr& e) {
  return std::visit([](const auto& child) { return child.location; }, e.child);
}

Location location(const ExprRef& e) {
  return std::visit([](const auto& child) { return child.location; }, e->child);
}
};
