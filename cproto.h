/* $Id: cproto.h,v 4.18 2013/10/25 20:28:26 tom Exp $
 *
 * Declarations for C function prototype generator
 */
#ifndef CPROTO_H
#define CPROTO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef BISON_HAS_YYTNAME
#define BISON_HAS_YYTNAME 0
#endif

#ifndef YACC_HAS_YYNAME
#define YACC_HAS_YYNAME 0
#endif

#ifndef YACC_HAS_YYTOKS
#define YACC_HAS_YYTOKS 0
#endif

#ifndef YACC_HAS_YYTOKS_2
#define YACC_HAS_YYTOKS_2 0
#endif

#ifndef HAVE_LINK
#define HAVE_LINK 0
#endif

#ifndef HAVE_POPEN_PROTOTYPE
#define HAVE_POPEN_PROTOTYPE 0
#endif

#ifndef HAVE_LIBDBMALLOC
#define HAVE_LIBDBMALLOC 0
#endif

#ifndef HAVE_LIBDMALLOC
#define HAVE_LIBDMALLOC 0
#endif

#include <system.h>

#ifndef GCC_PRINTFLIKE
#if defined(GCC_PRINTF) && !defined(printf)
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var)	/*nothing */
#endif
#endif

#ifndef GCC_SCANFLIKE
#if defined(GCC_SCANF) && !defined(scanf)
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)	/*nothing */
#endif
#endif

#ifndef	GCC_NORETURN
#define	GCC_NORETURN		/* nothing */
#endif

#ifndef	GCC_UNUSED
#define	GCC_UNUSED		/* nothing */
#endif

#if HAVE_LIBDMALLOC || HAVE_LIBDBMALLOC || defined(DOALLOC)
#undef  NO_LEAKS
#define NO_LEAKS 1
#endif

#ifdef	lint
#define	NEW(type)	(type *)0
#else
#define	NEW(type)	(type *)xmalloc(sizeof(type))
#endif

#ifndef UCH
#define UCH(c)		((unsigned char)(c))
#endif

/* Useful constants (mainly to avoid problems balancing parentheses...) */
#define ELLIPSIS      "..."
#define PAREN_L       '('
#define PAREN_R       ')'
#define SQUARE_L      '['
#define SQUARE_R      ']'
#define CURL_L        '{'
#define CURL_R        '}'
#define	COMMENT_BEGIN "/*"
#define COMMENT_END   "*/"

/* Boolean type */
typedef char boolean;

/* Source file text */
typedef struct text {
    char text[MAX_TEXT_SIZE];	/* source text */
    long begin;			/* offset in temporary file */
} Text;

/* This is a list of function parameters. */
typedef struct parameter_list {
    struct parameter *first;	/* pointer to first parameter in list */
    struct parameter *last;	/* pointer to last parameter in list */
    long begin_comment;		/* begin offset of comment */
    long end_comment;		/* end offset of comment */
    char *comment;		/* comment at start of parameter list */
} ParameterList;

/* Declaration specifier flags */
#define DS_NONE 	0	/* default */
#define DS_EXTERN	1	/* contains "extern" specifier */
#define DS_STATIC	2	/* contains "static" specifier */
#define DS_CHAR 	4	/* contains "char" type specifier */
#define DS_SHORT	8	/* contains "short" type specifier */
#define DS_FLOAT	16	/* contains "float" type specifier */
#define DS_INLINE	32	/* contains "inline" specifier */
#define DS_JUNK 	64	/* we're not interested in this declaration */

/* This structure stores information about a declaration specifier. */
typedef struct decl_spec {
    unsigned short flags;	/* flags defined above */
    char *text;			/* source text */
    long begin;			/* offset in temporary file */
} DeclSpec;

/* Styles of function definitions */
#if OPT_LINTLIBRARY
#define FUNC_UNKNOWN		-1	/* unspecified */
#else
#define FUNC_UNKNOWN		0	/* unspecified (same as FUNC_NONE) */
#endif
#define FUNC_NONE		0	/* not a function definition */
#define FUNC_TRADITIONAL	1	/* traditional style */
#define FUNC_ANSI		2	/* ANSI style */
#define FUNC_BOTH		3	/* both styles */
typedef int FuncDefStyle;

/* This structure stores information about a declarator. */
typedef struct declarator {
    char *name;			/* name of variable or function */
    char *text;			/* source text */
    long begin;			/* offset in temporary file */
    long begin_comment;		/* begin offset of comment */
    long end_comment;		/* end offset of comment */
    FuncDefStyle func_def;	/* style of function definition */
    ParameterList params;	/* function parameters */
    boolean pointer;		/* TRUE if it declares a pointer */
    struct declarator *head;	/* head function declarator */
    struct declarator *func_stack;	/* stack of function declarators */
    struct declarator *next;	/* next declarator in list */
} Declarator;

/* This is a list of declarators. */
typedef struct declarator_list {
    Declarator *first;		/* pointer to first declarator in list */
    Declarator *last;		/* pointer to last declarator in list */
} DeclaratorList;

/* This structure stores information about a function parameter. */
typedef struct parameter {
    struct parameter *next;	/* next parameter in list */
    DeclSpec decl_spec;
    Declarator *declarator;
    char *comment;		/* comment following the parameter */
} Parameter;

/* parser stack entry type */
typedef union {
    Text text;
    DeclSpec decl_spec;
    Parameter *parameter;
    ParameterList param_list;
    Declarator *declarator;
    DeclaratorList decl_list;
} YYSTYPE;

/* Prototype styles */
#if OPT_LINTLIBRARY
#define PROTO_ANSI_LLIB		-2	/* form ANSI lint-library source */
#define PROTO_LINTLIBRARY	-1	/* form lint-library source */
#endif
#define PROTO_NONE		0	/* do not output any prototypes */
#define PROTO_TRADITIONAL	1	/* comment out parameters */
#define PROTO_ABSTRACT		2	/* comment out parameter names */
#define PROTO_ANSI		3	/* ANSI C prototype */
typedef int PrototypeStyle;

#define ansiLintLibrary() (proto_style == PROTO_ANSI_LLIB)
#define knrLintLibrary()  (proto_style == PROTO_LINTLIBRARY)
#define lintLibrary()     (knrLintLibrary() || ansiLintLibrary())

/* The role of a function declarator */
#define FUNC_OTHER	0	/* miscellaneous declaration */
#define FUNC_PROTO	1	/* prototype */
#define FUNC_DEF	2	/* function definition */
typedef int FuncDeclRole;

/* Prototype/function definition output formats */
#define FMT_OTHER		0	/* miscellaneous */
#define FMT_PROTO		1	/* prototype */
#define FMT_FUNC		2	/* function definition */
#define FMT_FUNC_COMMENT	3	/* func. def. with parameter comments */
typedef int FuncFormatType;

/* select scope of declarations to output */
#define SCOPE_STATIC	1	/* only output declarations with local scope */
#define SCOPE_EXTERN	2	/* only output declarations with global scope */
#define SCOPE_ALL	3	/* output all declarations */
typedef int Scope;

/* Prototype/function definition output format */
typedef struct func_format {
    const char *decl_spec_prefix;	/* output before declaration specifier */
    const char *declarator_prefix;	/* output before declarator name */
    const char *declarator_suffix;	/* output before '(' of parameter list */
    const char *first_param_prefix;	/* output before first parameter */
    const char *middle_param_prefix;	/* output before each subsequent parameter */
    const char *last_param_suffix;	/* output after last parameter */
} FuncFormat;

/* Program options */
extern boolean extern_out;
extern Scope scope_out;
extern boolean inline_out;
#if OPT_LINTLIBRARY
extern boolean types_out;
extern boolean lint_shadowed;
#endif
extern boolean variables_out;
extern boolean promote_param;
extern PrototypeStyle proto_style;
extern FuncDefStyle func_style;
extern boolean proto_macro;
extern boolean define_macro;
extern const char *macro_name;
extern boolean proto_comments;
extern boolean file_comments;
extern boolean quiet;
extern const char *func_directive;
extern unsigned num_inc_dir;
extern char **inc_dir;
extern FuncFormat fmt[4];

/* Global declarations */
extern char *progname;
extern int varargs_num;		/* supports varargs-comment */
extern char *varargs_str;	/* additional info, such as PRINTFLIKEnn */
extern unsigned extern_in;	/* supports "LINT_EXTERNnn" */
extern int do_tracking;		/* supports "-X" option */
extern int exitlike_func;	/* supports noreturn-attribute */
extern unsigned in_include;	/* current include-level */
extern int debug_trace;
extern char base_file[];

/* cproto.c */
#if HAVE_LIBDBMALLOC
extern void ExitProgram(int code);
#define exit(code) ExitProgram(code)
#endif
#if !HAVE_LIBDMALLOC
#ifdef NO_LEAKS
extern void *xRealloc(void *p, unsigned n, char *f, int l);
extern void *xMalloc(unsigned n, char *f, int l);
extern char *xStrdup(const char *s, char *f, int l);
#define xrealloc(p,n)       xRealloc(p, n, __FILE__, __LINE__)
#define xmalloc(n)          xMalloc(n, __FILE__, __LINE__)
#define xstrdup(s)          xStrdup(s, __FILE__, __LINE__)
#else
extern void *xrealloc(void *p, size_t n);
extern void *xmalloc(size_t n);
extern char *xstrdup(const char *src);
#endif
#endif /* !HAVE_LIBDMALLOC */
extern void put_error(void);
extern int is_path_sep(int ch);
extern char *trim_path_sep(char *s);

/* lintlibs.c */
#if OPT_LINTLIBRARY
extern void put_string(FILE *outf, const char *s);
extern void put_char(FILE *outf, int c);
extern void put_newline(FILE *outf);
extern void put_blankline(FILE *outf);
extern void put_padded(FILE *outf, const char *s);
extern void fmt_library(int code);
extern void begin_tracking(void);
extern int already_declared(char *name);
extern void track_in(void);
extern int want_typedef(void);
extern void begin_typedef(void);
extern void copy_typedef(const char *s);
extern void end_typedef(void);
extern void imply_typedef(const char *s);
extern char *implied_typedef(void);
extern void indent(FILE *outf);
extern int lint_ellipsis(Parameter * p);
#if OPT_LINTLIBRARY
extern void flush_varargs(void);
#else
#define flush_varargs()		/* nothing */
#endif
extern void ellipsis_varargs(Declarator * d);
extern char *supply_parm(int count);
extern int is_actual_func(Declarator * d);
extern void put_body(FILE *outf, DeclSpec * decl_spec, Declarator * declarator);
# ifdef NO_LEAKS
extern void free_lintlibs(void);
# endif
#else
#define put_string(fp,S)    fputs(S, fp)
#define put_char(fp,C)      fputc(C, fp)
#define put_padded(fp,S)    fprintf(fp, "%s ", S)
#define put_body(fp,s,d)    put_string(fp,";\n")
#define track_in()
#define begin_typedef()
#define copy_typedef()
#define end_typedef()
#define imply_typedef(s)
#define implied_typedef()   ((char *)0)
#endif

/* strkey.c */
extern char *strkey(char *src, const char *key);
extern void strcut(char *src, const char *key);

/* grammar.y */
extern boolean is_typedef_name(char *name);
extern char *cur_file_name(void);
extern unsigned cur_line_num(void);
extern FILE *cur_tmp_file(void);
extern void cur_file_changed(void);
extern long cur_begin_comment(void);
extern char *cur_text(void);
extern void pop_file(int closed);
extern void init_parser(void);
extern void process_file(FILE *infile, const char *name);
#ifdef NO_LEAKS
extern void free_lexer(void);
extern void free_parser(void);
#endif

/* workaround for one of the bugs in bison 1.875 */
#ifdef YYBISON
#define YYSTYPE YYSTYPE
#endif

#ifdef HAVE_MKSTEMP
#define call_mktemp(s) mkstemp(s)
#else
#define call_mktemp(s) mktemp(s)
#endif

#define type_realloc(type,ptr,size) \
		(((ptr) != 0) \
		 ? (type *) xrealloc(ptr, (size) * sizeof(type)) \
		 : (type *) xmalloc((size) * sizeof(type)))

#endif /* CPROTO_H */
