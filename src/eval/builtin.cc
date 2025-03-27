#include <cstdio>
#include <eval.h>
#include <gc.h>
#include <iostream>
#include <print>
#include <object.h>
#include <dlfcn.h>

#ifdef __linux__
  #define OS "linux"
#elif __APPLE__
  #define OS "darwin"
#else
  #error PLATFROM IS NOT SUPPORTED
#endif

#define MAKE_BFUNC_OBJ(fn) Object{\
  .type = ObjectType::BFUNC,\
  .child = BFunc{\
    .func = (fn)\
  }\
}

extern const ObjectRef OBJECT_NULL;

static ObjectRef bos(const std::vector<ObjectRef>&);
static ObjectRef blen(const std::vector<ObjectRef>&);
static ObjectRef blib(const std::vector<ObjectRef>&);
static ObjectRef btype_of(const std::vector<ObjectRef>&);
static ObjectRef bto_int(const std::vector<ObjectRef>&);
static ObjectRef bto_float(const std::vector<ObjectRef>&);
static ObjectRef bprint(const std::vector<ObjectRef>&);
static ObjectRef bprintln(const std::vector<ObjectRef>&);
static ObjectRef bpush(const std::vector<ObjectRef>&);
static ObjectRef bpop(const std::vector<ObjectRef>&);
static ObjectRef bslice(const std::vector<ObjectRef>&);
static ObjectRef bread_int(const std::vector<ObjectRef>&);
static ObjectRef bread_float(const std::vector<ObjectRef>&);
static ObjectRef bread_string(const std::vector<ObjectRef>&);

static Object BFUNC_LEN_OBJECT = MAKE_BFUNC_OBJ(blen);
static Object BFUNC_OS_OBJECT = MAKE_BFUNC_OBJ(bos);
static Object BFUNC_LIB_OBJECT = MAKE_BFUNC_OBJ(blib);
static Object BFUNC_TYPE_OF_OBJECT = MAKE_BFUNC_OBJ(btype_of);
static Object BFUNC_TO_INT_OBJECT = MAKE_BFUNC_OBJ(bto_int);
static Object BFUNC_TO_FLOAT_OBJECT = MAKE_BFUNC_OBJ(bto_float);
static Object BFUNC_PRINT_OBJECT = MAKE_BFUNC_OBJ(bprint);
static Object BFUNC_PRINTLN_OBJECT = MAKE_BFUNC_OBJ(bprintln);
static Object BFUNC_PUSH_OBJECT = MAKE_BFUNC_OBJ(bpush);
static Object BFUNC_POP_OBJECT = MAKE_BFUNC_OBJ(bpop);
static Object BFUNC_SLICE_OBJECT = MAKE_BFUNC_OBJ(bslice);
static Object BFUNC_READ_INT_OBJECT = MAKE_BFUNC_OBJ(bread_int);
static Object BFUNC_READ_FLOAT_OBJECT = MAKE_BFUNC_OBJ(bread_float);
static Object BFUNC_READ_STRING_OBJECT = MAKE_BFUNC_OBJ(bread_string);

extern const std::map<std::string, ObjectRef> builtinfns = {
  {"len", &BFUNC_LEN_OBJECT},
  {"os", &BFUNC_OS_OBJECT},
  {"lib", &BFUNC_LIB_OBJECT},
  {"type_of", &BFUNC_TYPE_OF_OBJECT},
  {"to_int", &BFUNC_TO_INT_OBJECT},
  {"to_float", &BFUNC_TO_FLOAT_OBJECT},
  {"print", &BFUNC_PRINT_OBJECT},
  {"println", &BFUNC_PRINTLN_OBJECT},
  {"push", &BFUNC_PUSH_OBJECT},
  {"pop", &BFUNC_POP_OBJECT},
  {"slice", &BFUNC_SLICE_OBJECT},
  {"read_int", &BFUNC_READ_INT_OBJECT},
  {"read_float", &BFUNC_READ_FLOAT_OBJECT},
  {"read_string", &BFUNC_READ_STRING_OBJECT},
};

static ObjectRef
blen(const std::vector<ObjectRef>& args) {
  if(args.size() != 1) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        1,
        args.size()
      )
    );
  }

  ObjectRef obj = args.front();
  int64_t value = 0;

  switch(obj->type) {
    case ObjectType::STRING:
      value = std::get<String>(obj->child).value.length();
      break;

    case ObjectType::ARRAY:
      value = std::get<Array>(obj->child).elements.size();
      break;

    default:
      return serr("type is not supported");
  }

  ObjectRef res = gc::alloc();
  res->type = ObjectType::INT;
  res->child = Int{
    .value = value
  };
  return res;
}

static ObjectRef
bos(const std::vector<ObjectRef>& args) {
  if(args.size() != 0) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        0,
        args.size()
      )
    );
  }

  ObjectRef res = gc::alloc();
  res->type = ObjectType::STRING;
  res->child = String{
    .value = OS
  };
  return res;
}

static ObjectRef
blib(const std::vector<ObjectRef>& args) {
  if(args.size() != 1) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        1,
        args.size()
      )
    );
  }

  ObjectRef obj = args.front();
  if(obj->type != ObjectType::STRING) {
    return serr("expected string type as argument");
  }

  const std::string& libname = std::get<String>(obj->child).value;
  void* lib = dlopen(libname.c_str(), RTLD_LAZY);
  if(!lib) {
    return serr(
      std::format(
        "failed to load '{}' libary",
        libname
      )
    );
  }

  ObjectRef res = gc::alloc();
  res->type = ObjectType::ELIB;
  res->child = ELib{
    .lib = lib
  };
  return res;
}

static ObjectRef
btype_of(const std::vector<ObjectRef>& args) {
  if(args.size() != 1) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        1,
        args.size()
      )
    );
  }

  const ObjectRef& obj = args.front();
  std::string value = to_string(obj->type);
  if(obj->type == ObjectType::STRUCTVAL) {
    value = std::get<Struct>(std::get<StructVal>(obj->child).parent->child).name;
  }

  ObjectRef res = gc::alloc();
  res->type = ObjectType::STRING;
  res->child = String{
    .value = value
  };
  return res;
}

static ObjectRef
bto_int(const std::vector<ObjectRef>& args) {
  if(args.size() != 1) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        1,
        args.size()
      )
    );
  }

  ObjectRef obj = args.front();
  ObjectRef res = gc::alloc();
  res->type = ObjectType::INT;

  switch(obj->type) {
    case ObjectType::INT:
      return obj;

    case ObjectType::FLOAT:
      res->child = Int{
        .value = static_cast<int64_t>(std::get<Float>(obj->child).value)
      };
      return res;

    case ObjectType::BOOL:
      res->child = Int{
        .value = std::get<Bool>(obj->child).value
      };
      return res;

    default:
      return serr("type is not supported");
  }
}

static ObjectRef
bto_float(const std::vector<ObjectRef>& args) {
  if(args.size() != 1) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        1,
        args.size()
      )
    );
  }

  ObjectRef obj = args.front();
  ObjectRef res = gc::alloc();
  res->type = ObjectType::FLOAT;

  switch(obj->type) {
    case ObjectType::FLOAT:
      return obj;

    case ObjectType::INT:
      res->child = Float{
        .value = static_cast<double>(std::get<Int>(obj->child).value)
      };
      return res;

    default:
      return serr("type is not supported");
  }
}

static ObjectRef
bprint(const std::vector<ObjectRef>& args) {
  for(const auto& arg: args) {
    std::print("{}", arg->value());
  }
  return OBJECT_NULL;
}

static ObjectRef
bprintln(const std::vector<ObjectRef>& args) {
  bprint(args);
  std::print("\n");
  return OBJECT_NULL;
}

static ObjectRef
bpush(const std::vector<ObjectRef>& args) {
  if(args.size() != 2) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        2,
        args.size()
      )
    );
  }

  ObjectRef arr = args[0];
  ObjectRef obj = args[1];

  if(arr->type != ObjectType::ARRAY) {
    return serr("first argument must be 'array' type");
  }

  std::get<Array>(arr->child).elements.push_back(obj);
  return arr;
}

static ObjectRef
bpop(const std::vector<ObjectRef>& args) {
  if(args.size() != 1) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        1,
        args.size()
      )
    );
  }

  ObjectRef arr = args[0];

  if(arr->type != ObjectType::ARRAY) {
    return serr("expected 'array' type");
  }

  std::get<Array>(arr->child).elements.pop_back();
  return arr;
}

static ObjectRef
bslice(const std::vector<ObjectRef>& args) {
  if(args.size() > 0) {
    ObjectRef arr = args.front();
    if(arr->type != ObjectType::ARRAY) {
      return serr("exepected 'array' type");
    }

    switch(args.size()) {
      case 1: {
        ObjectRef cpy_arr = gc::alloc();
        cpy_arr->type = ObjectType::ARRAY;
        cpy_arr->child = Array{
          .elements = std::get<Array>(arr->child).elements
        };
        return cpy_arr;
      }

      case 3: {
        ObjectRef start_obj = args[1];
        if(start_obj->type != ObjectType::INT) {
          return serr("expected 'int' type as second argument");
        }

        ObjectRef end_obj = args[2];
        if(end_obj->type != ObjectType::INT) {
          return serr("expected 'int' type as third argument");
        }

        int64_t start = std::get<Int>(start_obj->child).value;
        int64_t end = std::get<Int>(end_obj->child).value;
        auto& elements = std::get<Array>(arr->child).elements;

        if(start < 0 || end > static_cast<int64_t>(elements.size()) || start >= end) {
          return serr("index out of range or invalid");
        }

        ObjectRef cpy_arr = gc::alloc();
        cpy_arr->type = ObjectType::ARRAY;
        cpy_arr->child = Array{
          .elements = std::vector<ObjectRef>(elements.begin() + start, elements.begin() + end)
        };
        return cpy_arr;
      }
    }
  }

  return
  serr(
    std::format(
      "function takes {} or {} arguments but {} were given",
      1,
      3,
      args.size()
    )
  );
}

static ObjectRef
bread_int(const std::vector<ObjectRef>& args) {
  if(args.size() != 0) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        0,
        args.size()
      )
    );
  }

  int64_t value = 0;
  std::cin >> value;
  ObjectRef res = gc::alloc();
  res->type = ObjectType::INT;
  res->child = Int{
    .value = value
  };
  return res;
}

static ObjectRef
bread_float(const std::vector<ObjectRef>& args) {
  if(args.size() != 0) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        0,
        args.size()
      )
    );
  }

  double value = 0;
  std::cin >> value;
  ObjectRef res = gc::alloc();
  res->type = ObjectType::FLOAT;
  res->child = Float{
    .value = value
  };
  return res;
}

static ObjectRef
bread_string(const std::vector<ObjectRef>& args) {
  if(args.size() != 0) {
    return serr(
      std::format(
        "function takes {} arguments but {} were given",
        0,
        args.size()
      )
    );
  }

  std::string value;
  std::cin.ignore();
  std::getline(std::cin, value);
  ObjectRef res = gc::alloc();
  res->type = ObjectType::STRING;
  res->child = String{
    .value = value
  };
  return res;
}
