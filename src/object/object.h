
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A wrapper for creating different primitive objects and maintain lifetime in eta
**  File: objects.h
*/

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>
#include <token.h>
#include <ast.h>

enum ObjectType {
  NULL_ = 0,
  INT,
  FLOAT,
  BOOL,
  STRING,
  ARRAY,
  RETVAL,
  SERR,
  DERR,
  FUNC,
  BFUNC,
  EFUNC,
  ELIB,
  __OBJECTTYPECOUNT__,
};

struct Object;

// class for managing object lifetime
class ObjectRef {
public:
  ObjectRef();
  ObjectRef(Object*);
  ObjectRef(const ObjectRef&);
  const ObjectRef& operator=(const ObjectRef&);
  explicit operator bool() const;
  const Object& operator*() const;
  Object* operator->();
  const Object* operator->() const;
  Object* get() const;
  ~ObjectRef();

private:
  Object* obj;
};

using BuiltinFn = std::function<ObjectRef(const std::vector<ObjectRef>&)>;

struct Null {
};

struct Int {
  int64_t value;
};

struct Float {
  double value;
};

struct Bool {
  bool value;
};

struct String {
  std::string value;
};

struct Array {
  std::vector<ObjectRef> elements;
};

struct RetVal {
  ObjectRef value;
};

struct Func {
  std::vector<ast::IdentifierRef> parameters;
  ast::BlockStmtRef body;
};

struct BFunc {
  BuiltinFn func;
};

struct EFunc {
  void* func;
  std::vector<token::Token> argstypes;
  token::Token rettype;
};

struct ELib {
  void* lib;
};

using ObjectChild = std::variant<
  Null,
  Int,
  Float,
  Bool,
  String,
  Array,
  RetVal,
  Func,
  BFunc,
  EFunc,
  ELib
>;

struct Object {
  ObjectType type;
  ObjectChild child;

  std::string value() const;
};

std::string to_string(ObjectType);

#endif
