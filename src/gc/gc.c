
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A ref counting garbage collector for eta
**  File: gc.c
*/

#include <objects.h>
#include <gc.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <uthash.h>

#ifndef GC_OBJ_THRESHOLD
  #define GC_OBJ_THRESHOLD 25
#endif

struct gcitem {
  struct object *key;
  size_t count;
  UT_hash_handle hh;
};

static struct gcitem *table;
static size_t itemcount = 0;

void
gc_init(void) {
  #ifdef GC_DEBUG_ENABLE
    printf("gc: init\n");
  #endif
  table = NULL;
}

struct object*
gc_alloc(void) {
  if(itemcount >= GC_OBJ_THRESHOLD) {
    gc_cleanup();
    itemcount = 0;
  }

  struct gcitem *item = (struct gcitem*)malloc(sizeof(struct gcitem));
  item->key = (struct object*)calloc(1, sizeof(struct object));
  item->count = 1;
  HASH_ADD_PTR(table, key, item);
  itemcount++;
  #ifdef GC_DEBUG_ENABLE
    printf("gc: allocated a new object\n\titemcount: %zu\n\tkey: %p\n", itemcount, item->key);
  #endif
  return item->key;
}

void
gc_borrow(struct object *item) {
  struct gcitem *gcitem;
  HASH_FIND_PTR(table, &item, gcitem);
  if(gcitem != NULL) {
    gcitem->count++;
    #ifdef GC_DEBUG_ENABLE
      printf("gc: borrowed an object\n\tkey: %p\n\tcount: %zu\n", item, gcitem->count);
    #endif
  }
}

void
gc_done(struct object *item) {
  struct gcitem *gcitem;
  HASH_FIND_PTR(table, &item, gcitem);
  if(gcitem != NULL && gcitem->count > 0) {
    gcitem->count--;
    #ifdef GC_DEBUG_ENABLE
      printf("gc: done\n\tkey: %p\n\tcount: %zu\n", item, gcitem->count);
    #endif
  }
}

size_t
gc_active_objs(void) {
  return HASH_COUNT(table);
}

void
gc_cleanup(void) {
  #ifdef GC_DEBUG_ENABLE
    printf("gc: applying object cleanup\n");
  #endif
  struct gcitem *curr, *tmp;
  HASH_ITER(hh, table, curr, tmp) {
    if(curr->count == 0) {
      #ifdef GC_DEBUG_ENABLE
        printf("\tgc: deallocated an object\n\tkey: %p\n\n", curr->key);
      #endif
      HASH_DEL(table, curr);
      object_destroy(curr->key);
      free(curr->key);
      free(curr);
    }
  }
}

void
gc_deinit(void) {
  #ifdef GC_DEBUG_ENABLE
    printf("gc: deinit\n");
  #endif
  struct gcitem *curr, *tmp;
  HASH_ITER(hh, table, curr, tmp) {
    #ifdef GC_DEBUG_ENABLE
      printf("\tgc: deallocated an object\n");
    #endif

    HASH_DEL(table, curr);
    object_destroy(curr->key);
    free(curr->key);
    free(curr);
  }
}
