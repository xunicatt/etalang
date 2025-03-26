#include <object.h>
#include <scope.h>
#include <string>

extern const ObjectRef OBJECT_NULL;

namespace gc {
  extern void done(Object*);
}

Scope::Scope() {
  outer = nullptr;
}

Scope::Scope(Scope* _outer) {
  outer = _outer;
}

bool
Scope::exists(const std::string& name) const {
  return table.contains(name);
}

bool
Scope::exists_any(const std::string& name) const {
  if(exists(name)) {
    return true;
  }

  if(outer != nullptr) {
    return outer->exists_any(name);
  }

  return false;
}

ObjectRef
Scope::get(const std::string& name) const {
  if(exists(name)) {
    return table.at(name);
  }

  if(outer != nullptr) {
    return outer->get(name);
  }

  return OBJECT_NULL;
}

ObjectRef
Scope::set(const std::string& name, const ObjectRef& obj) {
  table[name] = obj;
  return obj;
}

ObjectRef
Scope::update(const std::string& name, const ObjectRef& obj) {
  if(exists(name)) {
    table[name] = obj;
    return obj;
  }

  if(outer) {
    return outer->update(name, obj);
  }

  return OBJECT_NULL;
}
