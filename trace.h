/* $Id: trace.h,v 4.4 2011/01/02 18:59:05 tom Exp $ */

#ifndef TRACE_H
#define TRACE_H

#include <cproto.h>

extern void Where(char *f, int n);
extern void Trace(char *f, ...) GCC_PRINTFLIKE(1,2);
extern void show_alloc(void);
extern void Elapsed(char *msg);
extern void WalkBack(void);
extern void fail_alloc(char *msg, char *ptr);

#ifdef DOALLOC
extern void *doalloc(void *,unsigned);
extern void dofree(void *);

#define malloc(n)    doalloc(0,n)
#define realloc(p,n) doalloc(p,n)
#define free(n)      dofree(n)
#endif

#endif /* TRACE_H */
