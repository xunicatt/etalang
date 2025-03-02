
/*
**  Author: Aniket Biswas (xunicatt)
**  Github: https://github.com/xunicatt
**  Description: A ref counting garbage collector for eta
**  File: gc.h
*/

#ifndef __GC_H__
#define __GC_H__

#include <objects.h>

void gc_init(void);
struct object* gc_alloc(void);
void gc_borrow(struct object *);
void gc_done(struct object *);
void gc_cleanup(void);
size_t gc_active_objs(void);
void gc_deinit(void);

#endif
