/* $Id: cproto.c,v 4.16 2005/12/08 23:17:44 tom Exp $
 *
 * C function prototype generator and function definition converter
 */
#define VERSION "4.7e"

#include <stdio.h>
#include <ctype.h>
#include "cproto.h"

/* getopt declarations */
#if HAVE_GETOPT_H
#include <getopt.h>
#else
extern int getopt (int argc, char *const *argv, const char *shortopts);
extern char *optarg;
extern int optind;
#endif

/* Name of the program (from argv[0]) */
char *progname;

/* Program options */

/* If nonzero, output variables declared "extern" in include-files */
int extern_in = 0;

/* When TRUE, track the include-level (works with gcc, not some others) */
#if OPT_LINTLIBRARY
int do_tracking = FALSE;
#endif

/* When TRUE, suppress return-statements in function-bodies */
int exitlike_func = FALSE;

/* If TRUE, output "extern" before global declarations */
boolean extern_out = FALSE;

/* By default, generate global declarations only */
Scope scope_out = SCOPE_EXTERN;

/* If TRUE, output definitions for inline functions */
boolean inline_out = FALSE;

/* If TRUE, export typedef declarations (needed for lint-libs) */
#if OPT_LINTLIBRARY
boolean types_out = FALSE;
#endif

/* If TRUE, undef functions to avoid shadowing problems */
#if OPT_LINTLIBRARY
boolean lint_shadowed = FALSE;
#endif

/* If TRUE, generate variable declarations */
boolean variables_out = FALSE;

/* If TRUE, enable formal parameter promotion */
boolean promote_param = TRUE;

/* Style of function prototype to generate */
PrototypeStyle proto_style = PROTO_ANSI;

/* Function definition style converted to */
FuncDefStyle func_style = FUNC_UNKNOWN;

/* If TRUE, put guard macro around prototype parameters */
boolean proto_macro = FALSE;

/* Name of macro to guard prototypes */
char *macro_name = "P_";

/* If TRUE, output prototype macro definition */
boolean define_macro = TRUE;

/* If TRUE, output comments in prototypes */
boolean proto_comments = FALSE;

/* If TRUE, output comments naming source files */
boolean file_comments = TRUE;

/* Conditional compilation directive output in front of function definitions */
char *func_directive = "#ifdef ANSI_FUNC";

/* Output formats for function declarators */
FuncFormat fmt[] = {
    /* miscellaneous function declarator */
    { "", " ", "", "", " ", "" },
    /* prototype */
    { "", " ", "", "", " ", "" },
    /* function definition */
    { "", "\n", " ", "", " ", "" },
    /* function definition with parameter comments */
    { "", "\n", " ", "\n    ", "\n    ", "\n" },
};

/* If TRUE, don't output message if unable to read an include file */
boolean quiet = FALSE;

/* Include file directories */
int num_inc_dir = 0;
char **inc_dir = 0;

/* Run the C preprocessor */
#ifdef CPP
# if !HAVE_POPEN_PROTOTYPE
extern FILE *popen (const char *c, const char *m);
# endif
extern int pclose (FILE *p);
static char *cpp = CPP, *cpp_opt, *cpp_cmd;
#endif

/* Try to allocate some memory.
 * If unsuccessful, output an error message and exit.
 */
#if !HAVE_LIBDMALLOC
#ifdef NO_LEAKS
void *xMalloc(unsigned n, char *f, int l)
#else
void *xmalloc (unsigned n)
#endif
{
    void *p;
#if HAVE_LIBDBMALLOC
    p = debug_malloc(f, l, n);
#else
    p = malloc(n);
#endif

    if (p == NULL) {
	fprintf(stderr, "%s: out of memory (cannot allocate %u bytes)\n",
		progname, n);
	exit(EXIT_FAILURE);
    }
    return p;
}
#endif /* if !HAVE_LIBDMALLOC */

/* Try to reallocate some memory.
 * If unsuccessful, output an error message and exit.
 */
#if !HAVE_LIBDMALLOC
#ifdef NO_LEAKS
void *xRealloc(void *p, unsigned n, char *f, int l)
#else
void *xrealloc (void *p, unsigned n)
#endif
{
#if HAVE_LIBDBMALLOC
    p = debug_malloc(f, l, p, n);
#else
    p = realloc(p, n);
#endif

    if (p == NULL) {
	fprintf(stderr, "%s: out of memory (cannot allocate %u bytes)\n",
		progname, n);
	exit(EXIT_FAILURE);
    }
    return p;
}
#endif /* if !HAVE_LIBDMALLOC */

/* Copy the string into allocated memory.
 * If unsuccessful, output an error message and exit.
 */
#if !HAVE_LIBDMALLOC
#ifdef NO_LEAKS
char *xStrdup(const char *src, char *f, int l)
#else
char *xstrdup (const char *src)
#endif
{
#if defined(NO_LEAKS)
    return strcpy(xMalloc(strlen(src)+1, f, l), src);
#else
    return strcpy(xmalloc(strlen(src)+1), src);
#endif
}
#endif /* if !HAVE_LIBDMALLOC */

/* Output the current source file name and line number.
 */
void
put_error (void)
{
    fprintf(stderr, "%s:%u: ", cur_file_name(), cur_line_num());
}

/* Scan for options from a string.
 */
static void
parse_options (char *src, int maxargc, int *pargc, char **argv)
{
    char *g, *p, c;
    int argc;

    argc = 0;
    g = xstrdup(src);
    c = *g;
    while (c != '\0' && argc < maxargc) {
	while (c == ' ' || c == '\t')
	    c = *++g;
	if (c == '\0')
	    break;
	argv[argc++] = g;

	p = g;
	for (;;) {
	    if (c == ' ' || c == '\t' || c == '\0') {
		*p = '\0';
		break;
	    } else if (c == '"') {
		for (;;) {
		    c = *++g;
		    if (c == '"') {
			c = *++g;
			break;
		    } else if (c == '\0') {
			break;
		    } else {
			*p++ = c;
		    }
		}
	    } else {
		*p++ = c;
		c = *++g;
	    }
	}
	if (c != '\0')
	    c = *++g;
    }

    *pargc = argc;
}

/* Replace any character escape sequences in a string with the actual
 * characters.	Return a pointer to malloc'ed memory containing the result.
 * This function knows only a few escape sequences.
 */
static char *
escape_string (char *src)
{
    char *result, *get, *put;

    result = xstrdup(src);
    put = result;
    get = src;
    while (*get != '\0') {
	if (*get == '\\') {
	    switch (*(++get)) {
	    case 'n':
		*put++ = '\n';
		++get;
		break;
	    case 's':
		*put++ = ' ';
		++get;
		break;
	    case 't':
		*put++ = '\t';
		++get;
		break;
	    default:
		if (*get != '\0')
		    *put++ = *get++;
	    }
	} else {
	    *put++ = *get++;
	}
    }
    *put = *get;
    return result;
}

/* Returns true iff the character is a path leaf separator
 */
int
is_path_sep (int ch)
{
#if defined(MSDOS) || defined(OS2)
    return ch == '/' || ch == '\\';
#else
    return ch == '/';
#endif
}

/* Trim any path name separator from the end of the string.
 * Return a pointer to the string.
 */
char *
trim_path_sep (char *s)
{
    int n;

    n = strlen(s);
    if (n > 0) {
	if (is_path_sep(s[n-1]))
	    s[n-1] = '\0';
    }
    return s;
}

/* Output usage message and exit.
 */
static void
usage (void)
{
    fprintf(stderr, "usage: %s [ option ... ] [ file ... ]\n", progname);
    fputs("Options:\n", stderr);
    fputs("  -a, -t           Convert function definitions to ANSI or traditional style\n", stderr);
    fputs("  -b               Rewrite function definitions in both styles\n", stderr);
    fputs("  -c               Enable comments in prototype parameters\n", stderr);
    fputs("  -e               Output \"extern\" keyword before global declarations\n", stderr);
    fputs("  -f n             Set function prototype style (0 to 3)\n", stderr);
#if OPT_LINTLIBRARY
    fputs("  -l               Generate output in lint-library style\n", stderr);
#endif
    fputs("  -o file          Redirect output to file\n", stderr);
    fputs("  -O file          Redirect errors to file\n", stderr);
    fputs("  -p               Disable formal parameter promotion\n", stderr);
    fputs("  -q               Disable include file read failure messages\n", stderr);
    fputs("  -s               Output static declarations also\n", stderr);
    fputs("  -S               Output static declarations only\n", stderr);
    fputs("  -i               Output inline declarations also\n", stderr);
#if OPT_LINTLIBRARY
    fputs("  -T               Output type definitions\n", stderr);
#endif
    fputs("  -v               Output variable declarations\n", stderr);
    fputs("  -x               Output variables and functions declared \"extern\"\n", stderr);
#if OPT_LINTLIBRARY
    fputs("  -X level         Limit externs to given include-level\n", stderr);
#endif
    fputs("  -m               Put macro around prototype parameters\n", stderr);
    fputs("  -M name          Set name of prototype macro\n", stderr);
    fputs("  -d               Omit prototype macro definition\n", stderr);
    fputs("  -P template      Set prototype format template \" int f (a, b)\"\n", stderr);
    fputs("  -F template      Set function definition format template \" int f (a, b)\"\n", stderr);
    fputs("  -C template      Set format for function definition with parameter comments\n", stderr);
    fputs("  -D name[=value]  Define C preprocessor symbol\n", stderr);
    fputs("  -U name          Undefine C preprocessor symbol\n", stderr);
    fputs("  -I directory     Add #include search directory\n", stderr);
    fputs("  -E command       Run specified C preprocessor command\n", stderr);
    fputs("  -E 0             Do not run any C preprocessor\n", stderr);
    fputs("  -V               Print version information\n", stderr);
    exit(EXIT_FAILURE);
}

#define CHUNK(n) (((n) | 7) + 1)

/*
 * CURRENT_DIR is the first element in the array, and the system includes
 * are the last.  Other -I options are inserted in order between the two.
 */
static void
add_inc_dir (char *src)
{
    unsigned have = CHUNK(num_inc_dir);
    unsigned need = CHUNK(num_inc_dir + 1);
    unsigned used = (need * sizeof(char *));
    char *save;

    if (inc_dir == 0) {
	inc_dir = (char **)malloc(used);
    } else if (have != need) {
	inc_dir = (char **)realloc(inc_dir, used);
    }

    switch (num_inc_dir) {
    case 0:
	/* FALLTHRU */
    case 1:
	inc_dir[num_inc_dir++] = trim_path_sep(xstrdup(src));
	break;
    default:
	save = inc_dir[--num_inc_dir];
	inc_dir[num_inc_dir++] = trim_path_sep(xstrdup(src));
	inc_dir[num_inc_dir++] = save;
	break;
    }
}

#ifdef	vms
static	char	*cpp_defines;
static	char	*cpp_include;
static	char	*cpp_undefns;

static void
add2list(char *dst, char *src)
{
    if (*dst)
	strcat(dst, ",");
    strcat(dst, src);
}

static void
add_option(char *keyword, char *src)
{
    if (*src)
	sprintf(cpp_opt + strlen(cpp_opt), " /%s=(%s)", keyword, src);
}
#endif	/* vms */

#ifdef QUOTE_POPEN_ARGS

/* Calculate length of string including shell quoting characters that
 * must be inserted to preserve the string when it is passed to /bin/sh.
 * On UNIX systems, popen() runs /bin/sh.
 */
#define QUOTECHARS "\"\'\t\n "

static int
quote_length (char *s)
{
    int len = strlen(s);

    if (strpbrk(s, QUOTECHARS))  {
	len += 2;
	while (*s)
	    if (*s++ == '\'')
		len += 4; /* replace ' with '"'"' (ick!) */
    }
    return len;
}

/* Insert quoting characters to preserve the string when it is passed to
 * /bin/sh.
 */
static char *
quote_string (char *s)
{
    if (strpbrk(s, QUOTECHARS))  {
	char *src = s;
	char *dup, *dup_orig;
	
	dup = dup_orig = xstrdup(s);

	while (isspace(*src))
	    *src++ = *dup++;

	*src++ = '\'';
	while (*dup) {
	    if (*dup == '\'') {
		*src++ = '\'';
		*src++ = '"';
		*src++ = '\'';
		*src++ = '"';
		*src++ = '\'';
		dup++;
	    } else {
	        *src++ = *dup++;
	    }
	}

	*src++ = '\'';
	*src = '\0';
	free(dup_orig);
    }

    return s;
}
#else
#define	quote_length(s) strlen(s)
#define quote_string(s)	(s)
#endif /*QUOTE_POPEN_ARGS*/

#define MAX_OPTIONS 40

#define ALL_OPTIONS "\
B:\
C:\
D:\
E:\
F:\
I:\
M:\
O:\
P:\
S\
T\
U:\
V\
X:\
a\
b\
c\
d\
e\
f:\
i\
l\
m\
o:\
p\
q\
s\
t\
v\
x\
"
/* Process the command line options.
 */
static void
process_options (int *pargc, char ***pargv)
{
    int argc, eargc, nargc;
    char **argv, *eargv[MAX_OPTIONS], **nargv;
    int i, c;
    char *s;
#if defined(CPP)
    unsigned n;
#endif
#if defined(CPP) && !defined(vms)
    char *tmp;
#endif

    argc = *pargc;
    argv = *pargv;
#ifndef vms	/* this conflicts with use of foreign commands... */
    if ((s = getenv("CPROTO")) != NULL) {
	parse_options(s, MAX_OPTIONS, &eargc, eargv);
	nargv = (char **)xmalloc((eargc+argc+1)*sizeof(char *));
	nargv[0] = argv[0];
	nargc = 1;
	for (i = 0; i < eargc; ++i)
	    nargv[nargc++] = eargv[i];
	for (i = 1; i < argc; ++i)
	    nargv[nargc++] = argv[i];
	nargv[nargc] = NULL;
	argc = nargc;
	argv = nargv;
    }
#endif

#ifdef CPP
    /* Allocate buffer for C preprocessor command line. */
    n = strlen(cpp) + 1;
    for (i = 0; i < argc; ++i) {
	n += quote_length(argv[i]) + 1;  /* add more for possible quoting */
    }
#ifdef	vms
    *(cpp_include = xmalloc(n+argc)) = '\0';
    *(cpp_defines = xmalloc(n+argc)) = '\0';
    *(cpp_undefns = xmalloc(n+argc)) = '\0';
    n += 30;	/* for keywords */
#endif
    *(cpp_opt = xmalloc(n)) = '\0';
    n += (2 + strlen(CPP) + BUFSIZ);
    *(cpp_cmd = xmalloc(n)) = '\0';
#endif

    while ((c = getopt(argc, argv, ALL_OPTIONS)) != EOF) {
	switch (c) {
	case 'I':
#ifdef	vms
	    add2list(cpp_include, optarg);
	    break;
#else	/* unix */
	    add_inc_dir(optarg);
#endif
		/*FALLTHRU*/
	case 'D':
#ifdef	vms
	    add2list(cpp_defines, optarg);
	    break;
#endif
		/*FALLTHRU*/
	case 'U':
#ifdef	vms
	    add2list(cpp_undefns, optarg);
	    break;
#else	/* UNIX, etc. */
#ifdef CPP
	    tmp = xmalloc(quote_length(optarg) + 10);
	    sprintf(tmp, " -%c%s", c, optarg);
	    strcat(cpp_opt, quote_string(tmp));
	    free(tmp);
#endif
#endif
	    break;

	case 'a':
	    func_style = FUNC_ANSI;
	    break;
	case 'B':
	    func_directive = optarg;
	    break;
	case 'b':
	    func_style = FUNC_BOTH;
	    break;
	case 'c':
	    proto_comments = TRUE;
	    break;
	case 'd':
	    define_macro = FALSE;
	    break;
	case 'E':
#ifdef CPP
	    if (strcmp(optarg, "0") == 0) {
		cpp = NULL;
	    } else {
		cpp = optarg;
	    }
#endif
	    break;
	case 'e':
	    extern_out = TRUE;
	    break;
	case 'C':
	case 'F':
	case 'P':
	    s = escape_string(optarg);
	    i = (c == 'C') ? FMT_FUNC_COMMENT :
		((c == 'F') ? FMT_FUNC : FMT_PROTO);

	    fmt[i].decl_spec_prefix = s;
	    while (*s != '\0' && isascii(UCH(*s)) && !isalnum(UCH(*s))) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && isascii(UCH(*s)) && isalnum(UCH(*s))) ++s;
	    if (*s == '\0') usage();

	    fmt[i].declarator_prefix = s;
	    while (*s != '\0' && isascii(UCH(*s)) && !isalnum(UCH(*s))) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && isascii(UCH(*s)) && isalnum(UCH(*s))) ++s;
	    if (*s == '\0') usage();

	    fmt[i].declarator_suffix = s;
	    while (*s != '\0' && *s != '(') ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';

	    fmt[i].first_param_prefix = s;
	    while (*s != '\0' && isascii(UCH(*s)) && !isalnum(UCH(*s))) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && *s != ',') ++s;
	    if (*s == '\0') usage();

	    fmt[i].middle_param_prefix = ++s;
	    while (*s != '\0' && isascii(UCH(*s)) && !isalnum(UCH(*s))) ++s;
	    if (*s == '\0') usage();
	    *s++ = '\0';
	    while (*s != '\0' && isascii(UCH(*s)) && isalnum(UCH(*s))) ++s;
	    if (*s == '\0') usage();

	    fmt[i].last_param_suffix = s;
	    while (*s != '\0' && *s != ')') ++s;
	    *s = '\0';

	    break;
	case 'f':
	    proto_style = atoi(optarg);
	    if (proto_style < 0 || proto_style > PROTO_ANSI)
		usage();
	    break;
	case 'm':
	    proto_macro = TRUE;
	    break;
	case 'M':
	    macro_name = optarg;
	    break;
	case 'p':
	    promote_param = FALSE;
	    break;
	case 'q':
	    quiet = TRUE;
	    break;
	case 'S':
	    scope_out = SCOPE_STATIC;
	    break;
	case 's':
	    scope_out = SCOPE_ALL;
	    break;
	case 'i':
	    inline_out = TRUE;
	    break;
	case 't':
	    func_style = FUNC_TRADITIONAL;
	    break;
	case 'V':
	    fprintf(stderr, "%s\n", VERSION);
	    exit(EXIT_FAILURE);
	    break;
	case 'v':
	    variables_out = TRUE;
	    break;
	case 'o':
	    if (freopen(optarg, "w", stdout) == 0) {
		perror(optarg);
		exit(EXIT_FAILURE);
  	    }
	    break;
	case 'O':
	    if (freopen(optarg, "w", stderr) == 0) {
		perror(optarg);
		exit(EXIT_FAILURE);
  	    }
	    break;
#if OPT_LINTLIBRARY
	case 'T':	/* emit typedefs */
	    types_out = TRUE;
	    break;
	case 'l':
	    proto_style   = PROTO_LINTLIBRARY;
	    extern_out    = FALSE;
	    types_out     = TRUE;
	    variables_out = TRUE;
# if !defined(vms) && !defined(MSDOS)
	    (void)strcat(cpp_opt, " -C");	/* pass-through comments */
# endif
	    break;
	case 'X':
	    extern_in = atoi(optarg);
	    do_tracking = TRUE;
	    if (extern_in < 0)
		usage();
	    break;
#endif	/* OPT_LINTLIBRARY */
	case 'x':
	    extern_in = MAX_INC_DEPTH;
	    break;
	default:
	    usage();
	}
    }

#ifdef	vms
    add_option("includes", cpp_include);
    add_option("define",   cpp_defines);
    add_option("undefine", cpp_undefns);
#endif

    *pargc = argc;
    *pargv = argv;
}

int
main (int argc, char *argv[])
{
    int i;
    FILE *inf;
    char *argv0;

#ifdef __EMX__
    /* Expand file wild cards. */
    _wildcard(&argc, &argv);
#endif

    add_inc_dir(CURRENT_DIR);
#if defined(MSDOS) && defined(__TURBOC__)
    add_inc_dir("/tc/include");
#elif defined(vms)
    add_inc_dir("sys$library:");
#else
    add_inc_dir("/usr/include");
#endif

    /* Get the program name from the 0th argument, stripping the pathname
     * for readability.
     */
    progname = argv0 = xstrdup(argv[0]);
#ifdef vms
    for (i = strlen(progname)-1; i >= 0; i--) {
    	if (progname[i] == SQUARE_R
	 || progname[i] == ':') {
	    progname += (i + 1);
	    break;
	} else if (progname[i] == '.') {
	    progname[i] = '\0';
	}
    }
#else
    for (i = strlen(progname)-1; i >= 0; i--) {
	if (is_path_sep(progname[i])) {
	    progname += (i + 1);
	    break;
# if defined(MSDOS) || defined(OS2)
	} else if (progname[i] == '.') {
	    progname[i] = '\0';
# endif
	}
    }
#endif
    argv[0] = progname;	/* do this so getopt is consistent with us */

    process_options(&argc, &argv);

#if OPT_LINTLIBRARY
    if (lintLibrary()) {
    	put_string(stdout, "/* LINTLIBRARY */\n");
	switch (func_style) {
	case FUNC_ANSI:
	case FUNC_BOTH:
	    lint_shadowed = TRUE;	/* e.g., ctype.h */
	    proto_style = PROTO_ANSI_LLIB;
	    break;
	}
	func_style = FUNC_NONE;
    } else if (func_style == FUNC_UNKNOWN)
	func_style = FUNC_NONE;
#endif

    if (proto_macro && define_macro) {
	printf("#if __STDC__ || defined(__cplusplus)\n");
	printf("#define %s(s) s\n", macro_name);
	printf("#else\n");
	printf("#define %s(s) ()\n", macro_name);
	printf("#endif\n\n");
    }

    init_parser();
    if (optind == argc) {
	if (func_style != FUNC_NONE) {
	    proto_style = PROTO_NONE;
	    variables_out = FALSE;
	    file_comments = FALSE;
	}
	process_file(stdin, "stdin");
	pop_file(FALSE);
    } else {
	if (!optind)
	    optind++;
	for (i = optind; i < argc; ++i) {
#ifdef CPP
# if defined(CPP_DOES_ONLY_C_FILES)
	    /*
	     * GCC (and possibly other compilers) don't pass-through the ".l"
	     * and ".y" files to the C preprocessor stage.  Fix this by
	     * temporarily linking the input file to a temporary-file with a
	     * ".c" suffix.
	     *
	     * Of course, this solution assumes that the input directory is
	     * writeable.
	     */
	    char temp[BUFSIZ];
	    char *s = strcpy(temp, argv[i]);
#  if HAVE_LINK
	    int len = strlen(temp);
	    s += len - 1;
	    if ((len > 2)
	     && (s[-1] == '.')
	     && (*s == 'l' || *s == 'y')) {
		while (s != temp && s[-1] != '/')
		    s--;
		(void)strcpy(s, "XXXXXX.c");
	    	call_mktemp(temp);
	    	if (link(argv[i], temp) < 0)
		    (void)strcpy(temp, argv[i]);
	    }
#  endif
#  define FileName temp
# else
#  define FileName argv[i]
#  ifdef vms
	    char temp[BUFSIZ];
	    (void)strcpy(temp, FileName);
#  endif
# endif
	    if (func_style == FUNC_NONE && cpp != NULL) {
#ifdef vms
		/*
		 * Assume the 'cpp' command contains a "%s" for the name of
		 * the file that we're writing to.
		 */
		sprintf(cpp_cmd, cpp,
			call_mktemp(strcpy(temp, "sys$scratch:XXXXXX.i")));
		sprintf(cpp_cmd + strlen(cpp_cmd), "%s %s", cpp_opt, FileName);
		system(cpp_cmd);
		inf = fopen(temp, "r");
#else
		sprintf(cpp_cmd, "%s%s %s", cpp, cpp_opt, FileName);
		inf = popen(cpp_cmd, "r");
#endif
		if (inf == NULL || ferror(inf) || feof(inf)) {
		    fprintf(stderr, "%s: error running %s\n", progname,
		     cpp_cmd);
		    continue;
		}
	    } else {
		if ((inf = fopen(argv[i], "r")) == NULL) {
		    fprintf(stderr, "%s: cannot read file %s\n", progname,
		     argv[i]);
		    continue;
		}
	    }
#else
	    if ((inf = fopen(argv[i], "r")) == NULL) {
		fprintf(stderr, "%s: cannot read file %s\n", progname, argv[i]);
		continue;
	    }
#endif
	    process_file(inf, argv[i]);
#ifdef CPP
	    if (func_style == FUNC_NONE && cpp != NULL) {
#ifdef vms
		fclose(inf);
#else
		pclose(inf);
#endif
#if defined(CPP_DOES_ONLY_C_FILES) || defined(vms)
		if (strcmp(argv[i], temp)) {
			(void)unlink(temp);
		}
		pop_file(TRUE);
#endif
	    } else {
		pop_file(FALSE);
	    }
#else	/* no CPP defined */
	    pop_file(FALSE);
#endif
	}
    }

    if (proto_macro && define_macro) {
	printf("\n#undef %s\n", macro_name);
    }

#ifdef NO_LEAKS
# ifdef CPP
    if (cpp_opt != 0) free(cpp_opt);
    if (cpp_cmd != 0) free(cpp_cmd);
#  ifdef vms
    if (cpp_include != 0) free(cpp_include);
    if (cpp_defines != 0) free(cpp_defines);
    if (cpp_undefns != 0) free(cpp_undefns);
#  endif
# endif
    if (inc_dir != 0) {
	while (--num_inc_dir >= 0) {
	    free(inc_dir[num_inc_dir]);
	}
	free(inc_dir);
	inc_dir = 0;
    }
    free_lexer();
    free_parser();
# if OPT_LINTLIBRARY
    free_lintlibs();
# endif

# ifdef DOALLOC
    show_alloc();
# endif
    free(argv0);
#endif

    return EXIT_SUCCESS;
}

/* Intercept 'exit()' for debugging.  (The Linux libc uses malloc/free in
 * 'exit()', so we cannot get a trace unless we resort to this hack ;-)
 */
#if HAVE_LIBDBMALLOC
#undef exit
void ExitProgram(int code)
{
    extern int malloc_errfd;	/* FIXME: should be in dbmalloc.h */
    malloc_dump(malloc_errfd);
    exit(code);
}
#endif
