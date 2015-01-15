
#ifndef __GC_H__
#define __GC_H__

int gc(void);
void* gc_malloc(long size);
void gc_free(void);
int dbg_gc(void);

#endif ./* __GC_H__ */

