
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A ref counting garbage collector for eta
**  File: gc.h
*/

#ifndef __GC_H__
#define __GC_H__

#include <object.h>
#ifdef GC_DEBUG_INFO_ENABLE
  #include <cstddef>
#endif

namespace gc {
  ObjectRef alloc();
  Object* borrow(Object*);
  #ifdef GC_DEBUG_INFO_ENABLE
    void collect();
  #endif
  void done(Object*);
  #ifdef GC_DEBUG_INFO_ENABLE
    size_t ref(Object*);
    size_t count();
  #endif
  void purge();
};

#endif
