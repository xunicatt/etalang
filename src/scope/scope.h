
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: a special and important module to store named objects
**                i.e, variables
**  File: scope.h
*/

#ifndef __SCOPE_H__
#define __SCOPE_H__

#include <map>
#include <object.h>
#include <object.h>
#include <string>

class Scope {
public:
  Scope();
  Scope(Scope*);
  bool exists(const std::string&) const;
  bool exists_any(const std::string&) const;
  ObjectRef get(const std::string&) const;
  ObjectRef set(const std::string&, const ObjectRef&);
  ObjectRef update(const std::string&, const ObjectRef&);

private:
  std::map<std::string, ObjectRef> table;
  Scope* outer;
};

#endif
