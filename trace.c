/* $Id: trace.c,v 4.6 2011/01/02 19:28:44 tom Exp $
 *
 * Simple malloc debugging (for finding leaks)
 *
 * This is a cut-down version of a module I wrote originally for 'vile', it
 * requires an ANSI compiler.  Its main purpose is to allow tracing problems in
 * a repeatable test, including malloc/free bugs -- T.Dickey
 */
#include <trace.h>		/* interface of this module */

#if	DOALLOC
#undef	malloc
#undef	realloc
#undef	free
#endif /* DOALLOC */

#include <stdlib.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <stdarg.h>

#define	BEFORE	0		/* padding "before" allocated area */
#define AFTER   0		/* padding "after" allocated area */

#define BEFORE_PTR(ptr) (void *)((char *)(ptr) - BEFORE)

void
Where(char *file, int line)
{
    fflush(stderr);
    printf("%s @%d\n", file, line);
    fflush(stdout);
    Trace("%s @%d\n", file, line);
}

void
Trace(char *format,...)
{
    static FILE *fp;
    va_list ap;

    if (!fp)
	fp = fopen("Trace.out", "w");
    if (!fp)
	abort();

    if (format != 0) {
	va_start(ap, format);
	vfprintf(fp, format, ap);
	va_end(ap);
	(void) fflush(fp);
    } else {
	(void) fclose(fp);
	(void) fflush(stdout);
	(void) fflush(stderr);
    }
}

#define	SECS(tv)	(tv.tv_sec + (tv.tv_usec / 1.0e6))

void
Elapsed(char *msg)
{
#if HAVE_GETTIMEOFDAY
    static struct timeval tv0, tv1;
    static struct timezone tz0, tz1;
    static int init;
    if (!init++)
	gettimeofday(&tv0, &tz0);
    gettimeofday(&tv1, &tz1);
    Trace("%10.6f %s\n", SECS(tv1) - SECS(tv0), msg);
    tv0 = tv1;
#endif
}

#ifdef	apollo
static int
contains(char *ref, char *tst)
{
    size_t len = strlen(ref);
    while (*tst) {
	if (!strncmp(ref, tst++, len))
	    return TRUE;
    }
    return FALSE;
}
#endif /* apollo */

void
WalkBack(void)
{
#ifdef	apollo
    static char *first = "\"WalkBack\"", *last = "\"unix_$main\"";
    auto FILE *pp;
    auto char bfr[BUFSIZ];
    auto int ok = FALSE;
    static int count;

    Trace("%s %d\n", first, ++count);
    sprintf(bfr, "/com/tb %d", getpid());
    if (!(pp = popen(bfr, "r")))
	perror(bfr);

    while (fgets(bfr, sizeof(bfr), pp)) {
	if (ok && contains(last, bfr))
	    break;
	else if (contains(first, bfr))
	    ok = TRUE;
	else if (ok)
	    Trace("%s", bfr);
    }
    (void) fclose(pp);
#endif /* apollo */
}

static long count_alloc, count_freed;

void
fail_alloc(char *msg, char *ptr)
{
    Trace("%s: %p\n", msg, ptr);
    Trace("allocs %ld, frees %ld\n", count_alloc, count_freed);
    WalkBack();
#ifdef NO_LEAKS
    show_alloc();
#endif
    Trace((char *) 0);
    abort();
}

#if	DOALLOC
#undef	malloc
#undef	realloc
#undef	free

typedef struct {
    long size;			/* ...its size */
    char *text;			/* the actual segment */
    int note;			/* ...last value of 'count_alloc' */
} AREA;

static AREA area[DOALLOC];

static long maxAllocated,	/* maximum # of bytes allocated */
  nowAllocated,			/* current # of bytes allocated */
  nowPending,			/* current end of 'area[]' table */
  maxPending;			/* maximum # of segments allocated */

static int
FindArea(char *ptr)
{
    register int j;
    for (j = 0; j < DOALLOC; j++)
	if (area[j].text == ptr) {
	    if (j >= nowPending) {
		nowPending = j + 1;
		if (nowPending > maxPending)
		    maxPending = nowPending;
	    }
	    return j;
	}
    return -1;
}

static int
record_freed(char *ptr)
{
    register int j;
    if ((j = FindArea(ptr)) >= 0) {
	nowAllocated -= area[j].size;
	/*memset(ptr, 0xdf, area[j].size); -* poison */
	area[j].size = 0;
	area[j].text = 0;
	area[j].note = count_freed;
	if ((j + 1) == nowPending) {
	    register int k;
	    for (k = j; (k >= 0) && !area[k].size; k--)
		nowPending = k;
	}
    }
    return j;
}

static int
record_alloc(char *newp, char *oldp, unsigned len)
{
    register int j;

    if (newp == oldp) {
	if ((j = FindArea(oldp)) >= 0) {
	    nowAllocated -= area[j].size;
	    area[j].size = len;
	    area[j].note = count_alloc;
	} else
	    fail_alloc("could not find", oldp);
    } else {
	if (oldp != 0)
	    record_freed(oldp);
	if ((j = FindArea((char *) 0)) >= 0) {
	    area[j].text = newp;
	    area[j].size = len;
	    area[j].note = count_alloc;
	} else
	    fail_alloc("no room in table", newp);
    }

    nowAllocated += len;
    if (nowAllocated > maxAllocated)
	maxAllocated = nowAllocated;
    return len;
}

#define	OK_ALLOC(p,q,n)	((p != 0) && (record_alloc(p,q,n) >= 0))
#define	OK_FREE(p)	((p != 0) && (record_freed(p) >= 0))
#else
#define	OK_ALLOC(p,q,n)	(p != 0)
#define	OK_FREE(p)	(p != 0)
#endif /* DOALLOC */

#ifdef	DEBUG2
#define	LOG_PTR(msg,num)	Trace("%s %p\n", msg, num)
#define	LOG_LEN(msg,num)	Trace("%s %d\n", msg, num)
#else
#define LOG_PTR(msg,num)
#define	LOG_LEN(msg,num)
#endif

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/
#if DOALLOC
void *
doalloc(void *oldp, unsigned amount)
{
    char *newp;

    if (oldp != 0)
	oldp = BEFORE_PTR(oldp);
    count_alloc += (oldp == 0);
#if 0
    if ((count_alloc > 99914 && count_alloc < 99920)) {
	Trace("doalloc #%d\n", count_alloc);
	WalkBack();
    }
#endif
    LOG_LEN("allocate", amount);
    LOG_PTR("  old = ", oldp);
    amount += (BEFORE + AFTER);	/* patch */

    newp = (oldp != 0) ? realloc(oldp, amount) : malloc(amount);
    if (!OK_ALLOC(newp, oldp, amount)) {
	perror("doalloc");
	fail_alloc("doalloc", oldp);
	/*NOT REACHED */
    }

    LOG_PTR("  new = ", newp);
    return (void *) (newp + BEFORE);
}

/*
 * Entrypoint so we can validate pointers
 */
void
dofree(void *oldp)
{
    oldp = BEFORE_PTR(oldp);
    count_freed++;
    LOG_PTR("dealloc ", oldp);

    if (OK_FREE(oldp)) {
	free(oldp);
	return;
    }

    fail_alloc("free (not found)", oldp);
}
#endif

void
show_alloc(void)
{
#if	DOALLOC
    static char *format = ".. %-24.24s %10ld\n";

    Trace("** allocator metrics:\n");
    Trace(format, "allocs:", count_alloc);
    Trace(format, "frees:", count_freed);
    {
	register int j, count = 0;
	register long total = 0;

	for (j = 0; j < nowPending; j++) {
	    if (area[j].text) {
		if (count++ < 10)
		    Trace("...%d) %ld bytes in alloc #%d:%p\n",
			  j,
			  area[j].size,
			  area[j].note,
			  area[j].text);
		total += area[j].size;
	    }
	}
	Trace(format, "total bytes allocated:", total);
	Trace(format, "current bytes allocated:", nowAllocated);
	Trace(format, "maximum bytes allocated:", maxAllocated);
	Trace(format, "segment-table length:", nowPending);
	Trace(format, "current # of segments:", (long) count);
	Trace(format, "maximum # of segments:", maxPending);
    }
#endif
}
