#include <cstdint>
#include <format>
#include <iterator>
#include <print>
#include <pthread.h>
#include <scope.h>
#include <ast.h>
#include <cstddef>
#include <object.h>
#include <lexer.h>
#include <eval.h>
#include <gc.h>
#include <sstream>
#include <string>
#include <token.h>
#include <utility>
#include <vector>
#include <dlfcn.h>
#include <ffi.h>

using ast::StmtType;
using ast::ExprType;
using token::Token;

static Object ONULL = Object{.type = ObjectType::NULL_, .child = Null{}};
static Object OTRUE = Object{.type = ObjectType::BOOL, .child = Bool{.value = true}};
static Object OFALSE = Object{.type = ObjectType::BOOL, .child = Bool{.value = false}};

extern const ObjectRef OBJECT_TRUE = &OTRUE;
extern const ObjectRef OBJECT_NULL = &ONULL;
extern const ObjectRef OBJECT_FALSE = &OFALSE;
extern const std::map<std::string, ObjectRef> builtinfns;

static ObjectRef unaray_not(const ObjectRef&);
static ObjectRef unaray_sub(const ObjectRef&);
static ObjectRef to_bool(bool);
static ObjectRef binary(Token, const Int&, const Int&);
static ObjectRef binary(Token, const Float&, const Float&);
static ObjectRef binary(Token, const String&, const String&);
static ObjectType token_to_object_type(Token);
static ffi_type* object_to_ffi_type(ObjectType);
static void* object_to_data_ptr(ObjectRef&);
static void object_set_child(ObjectRef&);

Eval::Eval(Lexer& lexer)
  :lexer(lexer) {}

ObjectRef
Eval::evaluate(const ast::ProgramRef& prgm, Scope& scp) {
  ObjectRef result = OBJECT_NULL;

  for(const auto& _stmt: prgm->stmts) {
    result = stmt(_stmt, scp);

    if(is_err(result)) {
      return result;
    }
  }

  return result;
}

// =====================================================================
// STATEMENTS EVALUATOR
// =====================================================================
ObjectRef
Eval::stmt(const ast::StmtRef& stmt, Scope& scp) {
  switch(stmt->type) {
    case StmtType::EXPRESSIONSTMT:
      return expr_stmt(std::get<ast::ExprStmt>(stmt->child), scp);

    case StmtType::LETSTMT:
      return let_stmt(std::get<ast::LetStmt>(stmt->child), scp);

    case StmtType::RETURNSTMT:
      return return_stmt(std::get<ast::ReturnStmt>(stmt->child), scp);

    case StmtType::IFSTMT:
      return if_stmt(std::get<ast::IfStmt>(stmt->child), scp);

    case StmtType::FORSTMT:
      return for_stmt(std::get<ast::ForStmt>(stmt->child), scp);

    case StmtType::FUNCTIONSTMT:
      return func_stmt(std::get<ast::FunctionStmt>(stmt->child), scp);

    case StmtType::STRUCTSTMT:
      return struct_stmt(std::get<ast::StructStmt>(stmt->child), scp);

    case StmtType::EXTERNSTMT:
      return extern_stmt(std::get<ast::ExternStmt>(stmt->child), scp);

    default:
      return OBJECT_NULL;
  }
}

ObjectRef
Eval::struct_stmt(ast::StructStmt& stmt, Scope& scp) {
  if(scp.exists(stmt.name->value) || builtinfns.contains(stmt.name->value)) {
    return derr(stmt.name->location, "redefinition of variable");
  }

  const std::string& name = stmt.name->value;
  std::map<std::string, std::string> fields;

  for(size_t i = 0; i < stmt.names.size(); i++) {
    fields[stmt.names[i]->value] = stmt.types[i]->value;
  }

  ObjectRef res = gc::alloc();
  res->type = ObjectType::STRUCT;
  res->child = Struct{
    .name = name,
    .fields = fields
  };

  return scp.set(name, res);
}

ObjectRef
Eval::let_stmt(const ast::LetStmt& stmt, Scope& scp) {
  if(scp.exists(stmt.name->value) || builtinfns.contains(stmt.name->value)) {
    return derr(stmt.name->location, "redefinition of variable");
  }

  ObjectRef value = expr(stmt.value, scp);
  if(auto err = derr(ast::location(stmt.value), value); is_err(err)) {
    return err;
  }

  return scp.set(stmt.name->value, value);
}

ObjectRef
Eval::return_stmt(const ast::ReturnStmt& stmt, Scope& scp) {
  ObjectRef retval = gc::alloc();
  retval->type = ObjectType::RETVAL;

  if(stmt.value != nullptr) {
    ObjectRef value = expr(stmt.value, scp);
    if(auto err = derr(ast::location(stmt.value), value); is_err(err)) {
      return err;
    }
    retval->child = RetVal{
      .value = value
    };
  } else {
    retval->child = RetVal{
      .value = OBJECT_NULL
    };
  }

  return retval;
}

ObjectRef
Eval::if_stmt(const ast::IfStmt& stmt, Scope& scp) {
  ObjectRef condval = expr(stmt.condition, scp);
  if(auto err = derr(ast::location(stmt.condition), condval); is_err(err)) {
    return err;
  }

  if(condval->type != ObjectType::BOOL) {
    return derr(ast::location(stmt.condition), "expected a boolean value");
  }

  Scope ifscp(&scp);
  if(std::get<Bool>(condval->child).value) {
    return block_stmt(stmt.consequence, ifscp);
  }

  if(stmt.alternative != nullptr) {
    return block_stmt(stmt.alternative, ifscp);
  }

  return OBJECT_NULL;
}

ObjectRef
Eval::for_stmt(const ast::ForStmt& _stmt, Scope& scp) {
  if(_stmt.body->stmts.size() == 0) {
    return OBJECT_NULL;
  }

  Scope for_header(&scp);
  if(_stmt.pre != nullptr) {
    ObjectRef res = stmt(_stmt.pre, for_header);
    if(is_err(res)) {
      return res;
    }
  }

  while(true) {
    if(_stmt.condition != nullptr) {
      ObjectRef condval = expr(_stmt.condition, for_header);
      if(auto err = derr(ast::location(_stmt.condition), condval); is_err(err)) {
        return err;
      }

      if(condval->type != ObjectType::BOOL) {
        return derr(location(_stmt.condition), "expected a boolean value");
      }

      if(!(std::get<Bool>(condval->child).value)) {
        break;
      }
    }

    Scope for_body(&for_header);
    ObjectRef value = block_stmt(_stmt.body, for_body);
    if(value->type == ObjectType::RETVAL || is_err(value)) {
      return value;
    }

    if(_stmt.post != nullptr) {
      ObjectRef res = expr(_stmt.post, for_header);
      if(auto err = derr(ast::location(_stmt.post), res); is_err(err)) {
        return err;
      }
    }
  }

  return OBJECT_NULL;
}

ObjectRef
Eval::func_stmt(ast::FunctionStmt& _stmt, Scope& scp) {
  ObjectRef func = gc::alloc();
  func->type = ObjectType::FUNC;
  func->child = Func{
    .parameters = std::move(_stmt.parameters),
    .body = std::move(_stmt.body),
  };

  return scp.set(_stmt.name->value, func);
}

ObjectRef
Eval::extern_stmt(const ast::ExternStmt& stmt, Scope& scp) {
  if(scp.exists(stmt.funcname->value)) {
    return derr(stmt.funcname->location, "redefinition of variable");
  }

  if(!scp.exists_any(stmt.libname->value)) {
    return derr(stmt.libname->location, "undefined identifier");
  }

  ObjectRef libobj = scp.get(stmt.libname->value);
  if(libobj->type != ObjectType::ELIB) {
    return derr(
      stmt.libname->location,
      std::format(
        "expected a 'external libary' type but got '{}' type",
        to_string(libobj->type)
      )
    );
  }

  const ELib& lib = std::get<ELib>(libobj->child);
  void* func = dlsym(lib.lib, stmt.funcname->value.c_str());
  if(char* err = dlerror(); err != nullptr) {
    return derr(
      stmt.funcname->location,
      std::format(
        "failed to get external function: {}",
        err
      )
    );
  }

  ObjectRef res = gc::alloc();
  res->type = ObjectType::EFUNC;
  res->child = EFunc{
    .func = func,
    .argstypes = stmt.argtypes,
    .rettype = stmt.rettype
  };

  scp.set(stmt.funcname->value, res);
  return res;
}

ObjectRef
Eval::expr_stmt(const ast::ExprStmt& stmt, Scope& scp) {
  return expr(stmt.expr, scp);
}

ObjectRef
Eval::block_stmt(const ast::BlockStmtRef& _stmt, Scope& scp) {
  ObjectRef result = OBJECT_NULL;

  for(const auto& s: _stmt->stmts) {
    result = stmt(s, scp);

    if(result->type == ObjectType::RETVAL || is_err(result)) {
      return result;
    }
  }

  return OBJECT_NULL;
}

// =====================================================================
// END
// =====================================================================


// =====================================================================
// EXPRESSIONS EVALUATOR
// =====================================================================
ObjectRef
Eval::expr(const ast::ExprRef& _expr, Scope& scp) {
  switch(_expr->type) {
    case ExprType::NULLLIT:
      return OBJECT_NULL;

    case ExprType::INTEGERLIT:
      return integer_lit(std::get<ast::IntegerLit>(_expr->child));

    case ExprType::FLOATLIT:
      return float_lit(std::get<ast::FloatLit>(_expr->child));

    case ExprType::BOOLLIT:
      return bool_lit(std::get<ast::BoolLit>(_expr->child));

    case ExprType::STRINGLIT:
      return string_lit(std::get<ast::StringLit>(_expr->child));

    case ExprType::ARRAYLIT:
      return array_lit(std::get<ast::ArrayLit>(_expr->child), scp);

    case ExprType::STRUCTLIT:
      return struct_lit(std::get<ast::StructLit>(_expr->child), scp);

    case ExprType::IDENTEXPR:
      return ident_expr(std::get<ast::Identifier>(_expr->child), scp);

    case ExprType::UNARYEXP:
      return unary_expr(std::get<ast::UnaryExpr>(_expr->child), scp);

    case ExprType::BINARYEXP:
      return binary_expr(std::get<ast::BinaryExpr>(_expr->child), scp);

    case ExprType::ASSIGNMENTEXP:
      return assignment_expr(std::get<ast::AssignmentExpr>(_expr->child), scp);

    case ExprType::OPASSIGNMENTEXP:
      return opassignment_expr(std::get<ast::OpAssignmentExpr>(_expr->child), scp);

    case ExprType::INDEXEXP:
      return index_expr(std::get<ast::IndexExpr>(_expr->child), scp);

    case ExprType::CALLEXP:
      return call_expr(std::get<ast::CallExpr>(_expr->child), scp);

    case ExprType::MEMBEREXP:
      return member_expr(std::get<ast::MemberExpr>(_expr->child), scp);

    default:
      return OBJECT_NULL;
  }
}

static ObjectRef
unaray_not(const ObjectRef& value) {
  if(value.get() == OBJECT_TRUE.get()) return OBJECT_FALSE;
  if(value.get() == OBJECT_FALSE.get()) return OBJECT_TRUE;
  if(value.get() == OBJECT_NULL.get()) return OBJECT_TRUE;
  return OBJECT_FALSE;
}

static ObjectRef
unaray_sub(const ObjectRef& value) {
  ObjectRef result = gc::alloc();
  result->type = value->type;

  switch(value->type) {
    case ObjectType::INT:
      result->child = Int{
        .value = -(std::get<Int>(value->child).value)
      };
      break;

    case ObjectType::FLOAT:
      result->child = Float{
        .value = -(std::get<Float>(value->child).value)
      };
      break;

    default:
      result->type = ObjectType::SERR;
      result->child = String{
        .value = "type is not supported"
      };
  }

  return result;
}

ObjectRef
Eval::unary_expr(const ast::UnaryExpr& _expr, Scope& scp) {
  ObjectRef value = expr(_expr.right, scp);
  if(auto err = derr(ast::location(_expr.right), value); is_err(err)) {
    return err;
  }

  switch(_expr.operator_) {
    case Token::NOT:
      value = unaray_not(value);
      break;

    case Token::SUB:
      value = unaray_sub(value);
      break;

    default:
      return derr(_expr.location, "unknown operator");
  }

  if(auto err = derr(ast::location(_expr.right), value); is_err(err)) {
    return err;
  }

  return value;
}

static ObjectRef
to_bool(bool value) {
  return value ? OBJECT_TRUE : OBJECT_FALSE;
}

static ObjectRef
binary(Token operator_, const Int& lvalue, const Int& rvalue) {
  ObjectRef result = gc::alloc();
  int64_t resval = 0;

  switch(operator_) {
    case Token::ADD:
      resval = lvalue.value + rvalue.value;
      break;

    case Token::SUB:
      resval = lvalue.value - rvalue.value;
      break;

    case Token::MUL:
      resval = lvalue.value * rvalue.value;
      break;

    case Token::DIV:
      resval = lvalue.value / rvalue.value;
      break;

    case Token::LES:
      return to_bool(lvalue.value < rvalue.value);

    case Token::LEE:
      return to_bool(lvalue.value <= rvalue.value);

    case Token::GRT:
      return to_bool(lvalue.value > rvalue.value);

    case Token::GRE:
      return to_bool(lvalue.value >= rvalue.value);

    case Token::EQL:
      return to_bool(lvalue.value == rvalue.value);

    case Token::NEQL:
      return to_bool(lvalue.value != rvalue.value);

    default:
      result->type = ObjectType::SERR;
      result->child = String{
        .value = "unknown operator"
      };
      return result;
  }

  result->type = ObjectType::INT;
  result->child = Int{
    .value = resval
  };
  return result;
}

static ObjectRef
binary(Token operator_, const Float& lvalue, const Float& rvalue) {
  ObjectRef result = gc::alloc();
  double resval = 0;

  switch(operator_) {
    case Token::ADD:
      resval = lvalue.value + rvalue.value;
      break;

    case Token::SUB:
      resval = lvalue.value - rvalue.value;
      break;

    case Token::MUL:
      resval = lvalue.value * rvalue.value;
      break;

    case Token::DIV:
      resval = lvalue.value / rvalue.value;
      break;

    case Token::LES:
      return to_bool(lvalue.value < rvalue.value);

    case Token::LEE:
      return to_bool(lvalue.value <= rvalue.value);

    case Token::GRT:
      return to_bool(lvalue.value > rvalue.value);

    case Token::GRE:
      return to_bool(lvalue.value >= rvalue.value);

    case Token::EQL:
      return to_bool(lvalue.value == rvalue.value);

    case Token::NEQL:
      return to_bool(lvalue.value != rvalue.value);

    default:
      result->type = ObjectType::SERR;
      result->child = String{
        .value = "unknown operator"
      };
      return result;
  }

  result->type = ObjectType::FLOAT;
  result->child = Float{
    .value = resval
  };
  return result;
}

static ObjectRef
binary(Token operator_, const String& lvalue, const String& rvalue) {
  ObjectRef result = gc::alloc();
  std::string resval;

  switch(operator_) {
    case Token::ADD:
      resval = lvalue.value + rvalue.value;
      break;

    case Token::LES:
      return to_bool(lvalue.value < rvalue.value);

    case Token::LEE:
      return to_bool(lvalue.value <= rvalue.value);

    case Token::GRT:
      return to_bool(lvalue.value > rvalue.value);

    case Token::GRE:
      return to_bool(lvalue.value >= rvalue.value);

    case Token::EQL:
      return to_bool(lvalue.value == rvalue.value);

    case Token::NEQL:
      return to_bool(lvalue.value != rvalue.value);

    default:
      result->type = ObjectType::SERR;
      result->child = String{
        .value = "unknown operator"
      };
      return result;
  }

  result->type = ObjectType::STRING;
  result->child = String{
    .value = resval
  };
  return result;
}

ObjectRef
Eval::binary_expr(const ast::BinaryExpr& _expr, Scope& scp) {
  ObjectRef lvalue = expr(_expr.left, scp);
  if(auto err = derr(ast::location(_expr.left), lvalue); is_err(err)) {
    return err;
  }

  ObjectRef rvalue = expr(_expr.right, scp);
  if(auto err = derr(ast::location(_expr.right), rvalue); is_err(err)) {
    return err;
  }

  if(lvalue->type == ObjectType::INT && rvalue->type == ObjectType::INT) {
    ObjectRef result = binary(
      _expr.operator_,
      std::get<Int>(lvalue->child),
      std::get<Int>(rvalue->child)
    );

    if(auto err = derr(_expr.location, result); is_err(result)) {
      return err;
    }

    return result;
  }

  if(lvalue->type == ObjectType::FLOAT && rvalue->type == ObjectType::FLOAT) {
    ObjectRef result = binary(
      _expr.operator_,
      std::get<Float>(lvalue->child),
      std::get<Float>(rvalue->child)
    );

    if(auto err = derr(_expr.location, result); is_err(result)) {
      return err;
    }

    return result;
  }

  if(lvalue->type == ObjectType::STRING && rvalue->type == ObjectType::STRING) {
    ObjectRef result = binary(
      _expr.operator_,
      std::get<String>(lvalue->child),
      std::get<String>(rvalue->child)
    );

    if(auto err = derr(_expr.location, result); is_err(result)) {
      return err;
    }

    return result;
  }

  if(lvalue->type != rvalue->type) {
    return derr(
      _expr.location,
      std::format(
        "type mismatch between '{}' and '{}'",
        to_string(lvalue->type),
        to_string(rvalue->type)
      )
    );
  }

  if(_expr.operator_ == Token::EQL) {
    return to_bool(lvalue.get() == rvalue.get());
  }

  if(_expr.operator_ == Token::NEQL) {
    return to_bool(lvalue.get() != rvalue.get());
  }

  return derr(_expr.location, "unknown operator");
}

ObjectRef
Eval::integer_lit(const ast::IntegerLit& i) {
  ObjectRef obj = gc::alloc();
  obj->type = ObjectType::INT;
  obj->child = Int{
    .value = i.value
  };
  return obj;
}

ObjectRef
Eval::float_lit(const ast::FloatLit& f) {
  ObjectRef obj = gc::alloc();
  obj->type = ObjectType::FLOAT;
  obj->child = Float{
    .value = f.value
  };
  return obj;
}

ObjectRef
Eval::bool_lit(const ast::BoolLit& b) {
  return b.value ? OBJECT_TRUE : OBJECT_FALSE;
}

ObjectRef
Eval::string_lit(const ast::StringLit& s) {
  ObjectRef obj = gc::alloc();
  obj->type = ObjectType::STRING;
  obj->child = String{
    .value = s.value
  };
  return obj;
}

ObjectRef
Eval::array_lit(const ast::ArrayLit& a, Scope& scp) {
  std::vector<ObjectRef> elements;
  for(size_t i = 0; i < a.elements.size(); i++) {
    ObjectRef element = expr(a.elements[i], scp);
    if(auto err = derr(ast::location(a.elements[i]), element); is_err(err)) {
      return err;
    }
    elements.push_back(element);
  }

  ObjectRef array = gc::alloc();
  array->type = ObjectType::ARRAY;
  array->child = Array{
    .elements = elements
  };
  return array;
}

ObjectRef
Eval::struct_lit(const ast::StructLit& s, Scope& scp) {
  ObjectRef obj = expr(s.name, scp);
  if(obj->type != ObjectType::STRUCT) {
    return derr(ast::location(s.name), "expected 'struct' type");
  }

  const Struct& struct_obj = std::get<Struct>(obj->child);
  std::map<std::string, ObjectRef> fields;
  for(size_t i = 0; i < s.names.size(); i++) {
    if(!struct_obj.fields.contains(s.names[i]->value)) {
      return derr(
        s.names[i]->location,
        std::format(
          "struct '{}' contains no field named '{}'",
          struct_obj.name,
          s.names[i]->value
        )
      );
    }

    ObjectRef val = expr(s.value[i], scp);
    if(auto err = derr(ast::location(s.value[i]), val); is_err(err)) {
      return err;
    }

    std::string type_name = to_string(val->type);
    if(val->type == ObjectType::STRUCTVAL) {
      type_name = std::get<Struct>(std::get<StructVal>(val->child).parent->child).name;
    }

    if(struct_obj.fields.at(s.names[i]->value) != type_name) {
      return derr(
        s.names[i]->location,
        std::format(
          "expected type '{}' but got '{}'",
          struct_obj.fields.at(s.names[i]->value),
          type_name
        )
      );
    }

    fields[s.names[i]->value] = val;
  }

  std::vector<std::string> uninit_f;
  for(const auto& [k, v]: struct_obj.fields) {
    if(!fields.contains(k)) {
      uninit_f.push_back(k);
    }
  }

  if(uninit_f.size() > 0) {
    std::stringstream ss;
    std::copy(
      uninit_f.begin(),
      uninit_f.end(),
      std::ostream_iterator<std::string>(ss, ", ")
    );

    return derr(
      ast::location(s.name),
      std::format(
        "uninitialized fields: '{}'",
        ss.str()
      )
    );
  }

  ObjectRef res = gc::alloc();
  res->type = ObjectType::STRUCTVAL;
  res->child = StructVal{
    .parent = obj,
    .fields = fields
  };
  return res;
}

ObjectRef
Eval::ident_expr(const ast::Identifier& ident, Scope& scp) {
  if(scp.exists_any(ident.value)) {
    return scp.get(ident.value);
  }

  if(builtinfns.contains(ident.value)) {
    return builtinfns.at(ident.value);
  }

  return derr(ident.location, "undefined identifier");
}

ObjectRef
Eval::assignment_member(const ast::MemberExpr& _expr, const ast::ExprRef& right, Scope& scp) {
  ObjectRef obj = expr(_expr.left, scp);
  if(obj->type != ObjectType::STRUCTVAL) {
    return derr(ast::location(_expr.left), "expected a 'struct' instance type");
  }

  StructVal& sv = std::get<StructVal>(obj->child);
  if(!sv.fields.contains(_expr.field->value)) {
    return derr(
      _expr.field->location,
      std::format(
        "struct '{}' has no field named '{}'",
        std::get<Struct>(sv.parent->child).name,
        _expr.field->value
      )
    );
  }

  ObjectRef val = expr(right, scp);
  if(val->type != sv.fields.at(_expr.field->value)->type) {
    return derr(
      _expr.field->location,
      std::format(
        "expected type '{}' but got '{}'",
        to_string(sv.fields.at(_expr.field->value)->type),
        to_string(val->type)
      )
    );
  }

  sv.fields[_expr.field->value] = val;
  return obj;
}

ObjectRef
Eval::assignment_ident(const ast::Identifier& ident,
  const ast::ExprRef& right,
  Scope& scp) {
  if(!scp.exists_any(ident.value)) {
    return derr(ident.location, "undefined identifier");
  }

  ObjectRef obj = scp.get(ident.value);
  if(obj->type != ObjectType::NULL_ && obj->type == ObjectType::ELIB) {
    return derr(ident.location, "a 'library' type variable cannot be reassigned");
  }

  if(obj->type == ObjectType::BFUNC ||
    obj->type == ObjectType::FUNC ||
    obj->type == ObjectType::EFUNC) {
    return derr(ident.location, "a 'function' type variable cannot be reassigned");
  }

  ObjectRef value = expr(right, scp);
  if(auto err = derr(ast::location(right), value); is_err(err)) {
    return err;
  }

  if(obj->type != ObjectType::NULL_ && value->type != obj->type) {
    return derr(
      ident.location,
      std::format(
        "value of type '{}' cannot be assigned to a variable type of '{}'",
        to_string(value->type),
        to_string(obj->type)
      )
    );
  }

  return scp.update(ident.value, value);
}

ObjectRef
Eval::assignment_array(ObjectRef& obj,
  const ast::ExprRef& index,
  const ast::ExprRef& right,
  Scope& scp
) {
  Array& a = std::get<Array>(obj->child);
  ObjectRef idx = expr(index, scp);
  if(auto err = derr(ast::location(index), idx); is_err(err)) {
    return err;
  }

  if(idx->type != ObjectType::INT) {
    return derr(ast::location(index), "expected a 'int' type");
  }

  int64_t i = std::get<Int>(idx->child).value;
  if(i < 0 || i >= static_cast<int64_t>(a.elements.size())) {
    return derr(ast::location(index), "index out of range");
  }

  ObjectRef value = expr(right, scp);
  if(auto err = derr(location(right), value); is_err(err)) {
    return err;
  }

  a.elements[i] = value;
  return obj;
}

ObjectRef
Eval::assignment_string(
  ObjectRef& obj,
  const ast::ExprRef& index,
  const ast::ExprRef& right,
  Scope& scp
) {
  String& s = std::get<String>(obj->child);
  ObjectRef idx = expr(index, scp);
  if(auto err = derr(ast::location(index), idx); is_err(err)) {
    return err;
  }

  if(idx->type != ObjectType::INT) {
    return derr(ast::location(index), "expected a 'int' type");
  }

  int64_t i = std::get<Int>(idx->child).value;
  if(i < 0 || i >= static_cast<int64_t>(s.value.length())) {
    return derr(ast::location(index), "index out of range");
  }

  ObjectRef value = expr(right, scp);
  if(auto err = derr(location(right), value); is_err(err)) {
    return err;
  }

  if(value->type != ObjectType::STRING) {
    return derr(
      ast::location(right),
      std::format("type '{}' cannot be assigned to a string", to_string(value->type))
    );
  }

  if(std::get<String>(value->child).value.length() != 1) {
    return derr(
      ast::location(right),
      "expected a string with length of '1'"
    );
  }

  s.value[i] = std::get<String>(value->child).value[0];
  return obj;
}

ObjectRef
Eval::assignment_expr(const ast::AssignmentExpr& _expr, Scope& scp) {
  switch(_expr.left->type) {
    case ExprType::IDENTEXPR:
      return assignment_ident(
        std::get<ast::Identifier>(_expr.left->child),
        _expr.right,
        scp
      );

    case ExprType::MEMBEREXP:
      return assignment_member(
        std::get<ast::MemberExpr>(_expr.left->child),
        _expr.right,
        scp
      );

    case ExprType::INDEXEXP: {
      const ast::IndexExpr& idxexpr = std::get<ast::IndexExpr>(_expr.left->child);
      ObjectRef left = expr(idxexpr.left, scp);
      if(auto err = derr(ast::location(idxexpr.left), left); is_err(err)) {
        return err;
      }

      if(left->type != ObjectType::ARRAY && left->type != ObjectType::STRING) {
        return derr(ast::location(idxexpr.left), "type cannot be indexed");
      }

      switch(left->type) {
        case ObjectType::ARRAY:
          return assignment_array(
            left,
            idxexpr.index,
            _expr.right,
            scp
          );

        case ObjectType::STRING:
          return assignment_string(
            left,
            idxexpr.index,
            _expr.right,
            scp
          );

        default:
          break;
      }
    }

    default:
      return derr(
        ast::location(_expr.left),
        "constant cannot be assigned with a value"
      );
  }
}

ObjectRef
Eval::opassignment_expr(ast::OpAssignmentExpr& _expr, Scope& scp) {
  if(_expr.left->type != ExprType::IDENTEXPR) {
    return derr(_expr.location, "expected an non-indexed variable");
  }

  ast::BinaryExpr bin_expr = {
    .location = _expr.location,
    .operator_ = _expr.operator_,
    .left = std::move(_expr.left),
    .right = std::move(_expr.right)
  };

  ObjectRef value = binary_expr(bin_expr, scp);

  _expr.left = std::move(bin_expr.left);
  _expr.right = std::move(bin_expr.right);

  if(is_err(value)) {
    return value;
  }

  const auto& ident = std::get<ast::Identifier>(_expr.left->child);
  ObjectRef obj = scp.get(ident.value);

  if(obj->type != value->type) {
    return derr(
      _expr.location,
      std::format(
        "type mismatch between '{}' and '{}'",
        to_string(obj->type),
        to_string(value->type)
      )
    );
  }

  return scp.update(ident.value, value);
}

ObjectRef
Eval::index_expr(const ast::IndexExpr& _expr, Scope& scp) {
  ObjectRef obj = expr(_expr.left, scp);
  if(auto err = derr(ast::location(_expr.left), obj); is_err(obj)) {
    return err;
  }

  ObjectRef idx = expr(_expr.index, scp);
  if(auto err = derr(ast::location(_expr.index), idx); is_err(err)) {
    return err;
  }

  if(idx->type != ObjectType::INT) {
    return derr(_expr.location, "expected an 'int' type");
  }

  int64_t i = std::get<Int>(idx->child).value;

  switch(obj->type) {
    case ObjectType::STRING: {
      const std::string& str = std::get<String>(obj->child).value;
      if(i < 0 || i >= static_cast<int64_t>(str.length())) {
        return derr(ast::location(_expr.index), "index out of range");
      }

      ObjectRef value = gc::alloc();
      value->type = ObjectType::STRING;
      value->child = String{
        .value = std::string{str[i]}
      };
      return value;
    }

    case ObjectType::ARRAY: {
      const auto arr = std::get<Array>(obj->child).elements;
      if(i < 0 || i >= static_cast<int64_t>(arr.size())) {
        return derr(ast::location(_expr.index), "index out of range");
      }

      ObjectRef value = gc::alloc();
      return arr[i];
    }

    default:
      return derr(ast::location(_expr.left), "type cannot be indexed");
  }
}

ObjectRef
Eval::func(
  const Func& _func,
  const Location& loc,
  const std::vector<ast::ExprRef>& params,
  Scope& scp
) {
  if(_func.parameters.size() != params.size()) {
    return derr(
      loc,
      std::format(
        "function takes {} arguments but {} were given",
        _func.parameters.size(),
        params.size()
      )
    );
  }

  Scope func_scope(&scp);
  for(size_t i = 0; i < params.size(); i++) {
    ObjectRef value = expr(params[i], scp);
    if(auto err = derr(ast::location(params[i]), value); is_err(err)) {
      return err;
    }

    func_scope.set(_func.parameters[i]->value, value);
  }

  return block_stmt(_func.body, func_scope);
}

ObjectRef
Eval::bfunc(
  const BFunc& _func,
  const Location& loc,
  const std::vector<ast::ExprRef>& params,
  Scope& scp
) {
  std::vector<ObjectRef> args;
  for(const auto& p: params) {
    ObjectRef value = expr(p, scp);
    if(auto err = derr(ast::location(p), value); is_err(err)) {
      return err;
    }
    args.push_back(value);
  }

  ObjectRef retval = _func.func(args);
  if(auto err = derr(loc, retval); is_err(err)) {
    return err;
  }

  return retval;
}

static ObjectType
token_to_object_type(Token t) {
  switch(t) {
    case Token::INT: return ObjectType::INT;
    case Token::FLOAT: return ObjectType::FLOAT;
    case Token::BOOL: return ObjectType::BOOL;
    case Token::STRING: return ObjectType::STRING;
    default: return ObjectType::NULL_;
  }
};

static ffi_type*
object_to_ffi_type(ObjectType t) {
  switch(t) {
    case ObjectType::INT: return &ffi_type_sint;
    case ObjectType::FLOAT: return &ffi_type_double;
    case ObjectType::BOOL: return &ffi_type_sint;
    case ObjectType::STRING: return &ffi_type_pointer;
    default: return &ffi_type_void;
  }
};

static void*
object_to_data_ptr(ObjectRef& obj) {
  switch(obj->type) {
    case ObjectType::INT: return &(std::get<Int>(obj->child).value);
    case ObjectType::FLOAT: return &(std::get<Float>(obj->child).value);
    case ObjectType::BOOL: return &(std::get<Bool>(obj->child).value);
    case ObjectType::STRING: return (void*)(std::get<String>(obj->child).value.c_str());
    default: return nullptr;
  }
};

static void
object_set_child(ObjectRef& obj) {
  switch(obj->type) {
    case ObjectType::INT: obj->child = Int{}; break;
    case ObjectType::FLOAT: obj->child = Float{}; break;
    case ObjectType::BOOL: obj->child = Bool{}; break;
    case ObjectType::STRING: obj->child = String{}; break;
    default: obj->child = Null{}; break;
  }
};

ObjectRef
Eval::efunc(
  const EFunc& _func,
  const Location& loc,
  const std::vector<ast::ExprRef>& params,
  Scope& scp
) {
  bool is_variadic = false;

  if(_func.argstypes.size() > 0 && _func.argstypes.back() == Token::VARIADIC) {
    is_variadic = true;
    if(params.size() < _func.argstypes.size() - 1) {
      return derr(
        loc,
        std::format(
          "function takes atleast {} arguments but {} were given",
          _func.argstypes.size() - 1,
          params.size()
        )
      );
    }
  } else if(_func.argstypes.size() != params.size()) {
    return derr(
      loc,
      std::format(
        "function takes {} arguments but {} were given",
        _func.argstypes.size(),
        params.size()
      )
    );
  }

  std::vector<ObjectRef> objs;
  std::vector<ffi_type*> ffi_types;
  std::vector<void*> c_ptr_value;
  std::vector<char*> char_ref_handler;

  for(size_t i = 0; i < params.size(); i++) {
    ObjectRef obj = expr(params[i], scp);
    if(auto err = derr(ast::location(params[i]), obj); is_err(err)) {
      return err;
    }

    if(obj->type == ObjectType::STRUCTVAL) {
      return derr(
        ast::location(params[i]),
        "passing 'struct' type to external function is not implemented yet"
      );
    }

    if(!is_variadic || (is_variadic && _func.argstypes.size() - 1 > i)) {
      if(token_to_object_type(_func.argstypes[i]) != obj->type) {
        return derr(
          ast::location(params[i]),
          std::format(
            "expected '{}' type but got '{}' type",
            to_string(token_to_object_type(_func.argstypes[i])),
            to_string(obj->type)
          )
        );
      }
    }

    objs.push_back(obj);
    ffi_types.push_back(object_to_ffi_type(obj->type));
    if(obj->type == ObjectType::STRING) {
      char_ref_handler.push_back((char*)object_to_data_ptr(obj));
      c_ptr_value.push_back(&char_ref_handler.back());
      continue;
    }

    c_ptr_value.push_back(object_to_data_ptr(obj));
  }

  ffi_status status;
  ffi_cif cif;
  ffi_type* ret_type = object_to_ffi_type(token_to_object_type(_func.rettype));
  if(is_variadic) {
    status = ffi_prep_cif_var(
      &cif,
      FFI_DEFAULT_ABI,
      _func.argstypes.size() - 1,
      params.size(),
      ret_type,
      ffi_types.data()
    );
  } else {
    status = ffi_prep_cif(
      &cif,
      FFI_DEFAULT_ABI,
      params.size(),
      ret_type,
      ffi_types.data()
    );
  }

  if(status != FFI_OK) {
    return derr(loc, "failed to ffi_prep_cif");
  }

  char* ret_str = nullptr;
  void* ret_val = nullptr;
  ObjectRef res = gc::alloc();

  res->type = token_to_object_type(_func.rettype);
  if(res->type == ObjectType::STRING) {
    ret_val = &ret_str;
  } else {
    object_set_child(res);
    ret_val = object_to_data_ptr(res);
  }

  ffi_call(&cif, FFI_FN(_func.func), ret_val, c_ptr_value.data());

  if(res->type == ObjectType::BOOL) {
    res = std::get<Bool>(res->child).value ? OBJECT_TRUE : OBJECT_FALSE;
  } else if(res->type == ObjectType::STRING) {
    res->child = String{
      .value = ret_str
    };
  }

  return res;
}

ObjectRef
Eval::call_expr(const ast::CallExpr& _expr, Scope& scp) {
  ObjectRef funcobj = expr(_expr.function, scp);
  if(auto err = derr(ast::location(_expr.function), funcobj); is_err(err)) {
    return err;
  }

  ObjectRef retval = OBJECT_NULL;
  switch(funcobj->type) {
    case ObjectType::FUNC:
      retval = func(
        std::get<Func>(funcobj->child),
        location(_expr.function),
        _expr.arguments,
        scp
      );
      break;

    case ObjectType::BFUNC:
      retval = bfunc(
        std::get<BFunc>(funcobj->child),
        ast::location(_expr.function),
        _expr.arguments,
        scp
      );
      break;

    case ObjectType::EFUNC:
      retval = efunc(
        std::get<EFunc>(funcobj->child),
        ast::location(_expr.function),
        _expr.arguments,
        scp
      );
      break;

    default:
      return derr(location(_expr.function), "not a function");
  }

  if(retval->type == ObjectType::RETVAL) {
    retval = std::get<RetVal>(retval->child).value;
  }

  return retval;
}

ObjectRef
Eval::member_expr(const ast::MemberExpr& _expr, Scope& scp) {
  ObjectRef obj = expr(_expr.left, scp);
  if(obj->type != ObjectType::STRUCTVAL) {
    return derr(ast::location(_expr.left), "expected a 'struct' instance type");
  }

  const StructVal& s = std::get<StructVal>(obj->child);
  if(!s.fields.contains(_expr.field->value)) {
    return derr(
      _expr.field->location,
      std::format(
        "struct '{}' has no field named '{}'",
        std::get<Struct>(s.parent->child).name,
        _expr.field->value
      )
    );
  }

  return s.fields.at(_expr.field->value);
}

bool
Eval::is_err(const ObjectRef& obj) {
  return (obj) && (obj->type == SERR || obj->type == DERR);
}

ObjectRef
serr(const std::string& err_msg) {
  ObjectRef err = gc::alloc();
  err->type = ObjectType::SERR;
  err->child = String{
    .value = err_msg
  };
  return err;
}

ObjectRef
Eval::derr(const Location& loc, const ObjectRef& obj) {
  if(!is_err(obj)) {
    return obj;
  }

  if(obj->type == ObjectType::DERR) {
    return obj;
  }

  return derr(loc, std::get<String>(obj->child).value);
}

ObjectRef
Eval::derr(const Location& loc, const std::string& err_msg) {
  ObjectRef err = gc::alloc();
  err->type = ObjectType::DERR;
  err->child = String {
    .value = lexer.fmt_error(loc, err_msg)
  };
  return err;
}
