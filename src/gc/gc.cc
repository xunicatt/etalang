#include <print>
#include <cstddef>
#include <gc.h>
#include <object.h>
#include <map>

namespace gc {
  static constexpr size_t MAX_OBJ_ALLOC = 30;
  static std::map<Object*, size_t> object_table;
  static size_t active_objects = 0;

  #ifdef GC_DEBUG_INFO_ENABLE
    void
  #else
    static void
  #endif
  collect() {
    for(auto it = object_table.begin(); it != object_table.end();) {
      if(it->second == 0) {
        delete it->first;
        it = object_table.erase(it);
        continue;
      }

      it++;
    }
  }

  ObjectRef alloc() {
    if(active_objects >= MAX_OBJ_ALLOC) {
      collect();
      active_objects = 0;
    }

    Object* obj = new Object;
    object_table[obj] = 0;
    active_objects++;
    return ObjectRef(obj);
  }

  Object* borrow(Object* o) {
    if(object_table.contains(o)) {
      object_table[o] = object_table[o] + 1;
    }
    return o;
  }

  void done(Object* o) {
    if(object_table.contains(o) && object_table[o] > 0) {
      object_table[o] = object_table[o] - 1;
    }
  }

  #ifdef GC_DEBUG_INFO_ENABLE
    size_t ref(Object* o) {
      if(object_table.contains(o)) {
        return object_table[o];
      }

      return -1;
    }

    size_t count() {
      return object_table.size();
    }
  #endif

  void purge() {
    for(auto it = object_table.begin(); it != object_table.end();) {
      delete it->first;
      it = object_table.erase(it);
    }
  }
};
