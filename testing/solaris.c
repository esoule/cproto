/* $Id: solaris.c,v 4.1 1994/10/12 14:12:48 cthuang Exp $ */
/*LINTLIBRARY*/
/*LINT_EXTERN*/
/*LINT_SHADOWED*/
#include <sys/types.h>
#include <netinet/in.h>
/*LINT_PREPRO2
#define MAXDNAME 10
*/
#define MAXDNAME 10

#include <aio.h>
#include <alloca.h>
#include <ar.h>
#include <archives.h>
#include <assert.h>
#include <cpio.h>
#include <crypt.h>
#include <ctype.h>
#include <curses.h>
#include <deflt.h>
#include <devmgmt.h>
/*#include <dial.h> -- true redef */
#include <dirent.h>
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <eti.h>
#include <euc.h>
#include <fatal.h>
#include <fcntl.h>
#include <float.h>
#include <floatingpoint.h>
#include <fmtmsg.h>
#include <form.h>
#include <ftw.h>
#include <getwidth.h>
#include <grp.h>
#include <ieeefp.h>
#include <kstat.h>
#include <kvm.h>
#include <langinfo.h>
#include <lastlog.h>
#include <libelf.h>
#include <libgen.h>
#include <libgenIO.h>
#include <libintl.h>
#include <libw.h>
#include <limits.h>
#include <linenum.h>
#include <link.h>
#include <listen.h>
#include <locale.h>
#include <macros.h>
#include <maillock.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <menu.h>
#include <mon.h>
#include <mp.h>
#include <mqueue.h>
#include <nan.h>
#include <ndbm.h>
#include <netconfig.h>
#include <netdb.h>
#include <netdir.h>
#include <nl_types.h>
#include <nlist.h>
#include <nss_common.h>
#include <nss_dbdefs.h>
#include <nsswitch.h>
#include <panel.h>
#include <pkgdev.h>
#include <pkginfo.h>
#include <pkglocs.h>
#include <pkgstrct.h>
#include <pkgtrans.h>
#include <poll.h>
#include <prof.h>
#include <pw.h>
#include <pwd.h>
/*#include <regexp.h>*/
#include <regexpr.h>
#include <resolv.h>
#include <rje.h>
#include <rmmount.h>
#include <sac.h>
#include <sched.h>
#include <search.h>
#include <semaphore.h>
#include <setjmp.h>
/*#include <sgtty.h> */
#include <shadow.h>
#include <siginfo.h>
#include <signal.h>
#include <std.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <storclass.h>
#include <string.h>
#include <stropts.h>
#include <sum.h>
#include <syms.h>
#include <synch.h>
#include <syslog.h>
/* #include <table.h> conflict with search.h */
#include <tar.h>
/* #include <term.h> conflict with termcap.h */
/*LINT_PREPRO2
#undef PC
*/
#undef PC
#include <termcap.h>
#include <termio.h>
#include <termios.h>
#include <thread.h>
#include <time.h>
#include <tiuser.h>
#include <tzfile.h>
#include <ucontext.h>
#include <ulimit.h>
#include <unctrl.h>
#include <unistd.h>
#include <userdefs.h>
#include <ustat.h>
#include <utime.h>
#include <utmp.h>
#include <utmpx.h>
#include <valtools.h>
#include <values.h>
/* #include <varargs.h> - gcc prefers to give its own include-pathname here */
#include <volmgt.h>
#include <wait.h>
#include <wctype.h>
#include <widec.h>
