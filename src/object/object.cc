#include "token.h"
#include <cstddef>
#include <format>
#include <object.h>
#include <string>
#include <print>

namespace gc {
  extern Object* borrow(Object*);
  extern void done(Object*);
};

static const char* const objectstr[__OBJECTTYPECOUNT__]  = {
  "null",
  "int",
  "float",
  "bool",
  "string",
  "array",
  "struct",
  "",
  "return value",
  "error",
  "error",
  "function",
  "builtin function",
  "external function",
  "external libray",
};

ObjectRef::ObjectRef() {
  obj = nullptr;
}

ObjectRef::ObjectRef(Object* o) {
  obj = gc::borrow(o);
}

ObjectRef::ObjectRef(const ObjectRef& o) {
  obj = gc::borrow(o.obj);
}

const ObjectRef&
ObjectRef::operator=(const ObjectRef& o) {
  if(obj != nullptr) {
    gc::done(obj);
  }

  obj = gc::borrow(o.obj);
  return *this;
}


ObjectRef::operator bool() const {
  return obj != nullptr;
}

const Object&
ObjectRef::operator*() const {
  if(obj == nullptr) {
    std::println(stderr, "ObjectRef: a nullptr was dereferenced");
    exit(1);
  }

  return *obj;
}

Object*
ObjectRef::operator->() {
  if(obj == nullptr) {
    std::println(stderr, "ObjectRef: a nullptr was dereferenced");
    exit(1);
  }

  return obj;
}

const Object*
ObjectRef::operator->() const {
  if(obj == nullptr) {
    std::println(stderr, "ObjectRef: a nullptr was dereferenced");
    exit(1);
  }

  return obj;
}

Object*
ObjectRef::get() const {
  if(obj == nullptr) {
    std::println(stderr, "ObjectRef: a nullptr was dereferenced");
    exit(1);
  }

  return obj;
}

ObjectRef::~ObjectRef() {
  gc::done(obj);
}

std::string
to_string(ObjectType t) {
  return objectstr[t];
}

std::string
to_string(const Null& n) {
  return "null";
}

std::string
to_string(const Int& i) {
  return std::format("{}", i.value);
}

std::string
to_string(const Float& f) {
  return std::format("{}", f.value);
}

std::string
to_string(const Bool& b) {
  return std::format("{}", b.value ? "true" : "false");
}

std::string
to_string(const String& s) {
  return s.value;
}

std::string
to_string(const Array& a) {
  std::string arrval = "[";
  for(size_t i = 0; i < a.elements.size(); i++) {
    arrval += (*a.elements[i]).value();
    arrval += i < a.elements.size() - 1 ? ", " : "";
  }
  arrval += "]";
  return arrval;
}

std::string
to_string(const Struct& s) {
  using token::Token;

  std::string fields = "{";
  size_t i = 0;
  for(const auto& [k, v]: s.fields) {
    fields += k + ": " + v;
    fields += i < s.fields.size() - 1 ? ", " : "";
    i++;
  }
  fields += "}";
  return s.name + fields;
}

std::string
to_string(const StructVal& s) {
  std::string fields = "{";
  size_t i = 0;
  for(const auto& [k, v]: s.fields) {
    fields += k + ": " + v->value();
    fields += i < s.fields.size() - 1 ? ", " : "";
    i++;
  }
  fields += "}";
  return std::get<Struct>(s.parent->child).name + fields;
}

std::string
to_string(const RetVal& rv) {
  return (*rv.value).value();
}

std::string
to_string(const Func& f) {
  return to_string(ObjectType::FUNC);
}

std::string
to_string(const BFunc& f) {
  return to_string(ObjectType::BFUNC);
}

std::string
to_string(const EFunc& f) {
  return to_string(ObjectType::EFUNC);
}

std::string
to_string(const ELib& l) {
  return to_string(ObjectType::ELIB);
}

std::string
Object::value() const {
  return std::visit([](const auto& child) { return to_string(child); }, child);
}
