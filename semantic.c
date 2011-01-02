/* $Id: semantic.c,v 4.15 2011/01/02 19:17:15 tom Exp $
 *
 * Semantic actions executed by the parser of the
 * C function prototype generator.
 */
#include <semantic.h>

#if OPT_LINTLIBRARY
#define	putParameter(fp,p,f,n,c)		put_parameter(fp, p, f, n, c)
#else
#define	putParameter(fp,p,f,n,c)		put_parameter(fp, p, c)
#endif

static void put_declarator(FILE *outf, Declarator * declarator, int commented);

/* Head function declarator in a prototype or function definition */
static Declarator *func_declarator;

/* Role of the function declarator
 * FUNC_PROTO if it is the declarator in a function prototype.
 * FUNC_DEF if it is the declarator in a function definition.
 */
static int where;

/* Output format to use */
static int format;

/* Function-parameter level, used to simplify logic that comments/suppresses
 * function parameters in the output.
 */
static int nestedParams;

/* Initialize a new declaration specifier part.
 */
void
new_decl_spec(DeclSpec * decl_spec, const char *text, long offset, int flags)
{
#if OPT_LINTLIBRARY
    if (lintLibrary()) {
	if (!strcmp(text, "register"))
	    text = "";
    }
#endif
    decl_spec->text = xstrdup(text);
    decl_spec->begin = offset;
    decl_spec->flags = (unsigned short) flags;
}

/* Free storage used by a declaration specifier part.
 */
void
free_decl_spec(DeclSpec * decl_spec)
{
    free(decl_spec->text);
}

/* Create a new string by joining two strings with a space between them.
 * Return a pointer to the resultant string.
 * If out of memory, output an error message and exit.
 */
static char *
concat_string(char *a, char *b)
{
    char *result;

    result = (char *) xmalloc(strlen(a) + strlen(b) + 2);
    strcpy(result, a);
    strcat(result, " ");
    strcat(result, b);
    return result;
}

#if OPT_LINTLIBRARY
/* concatenate w/o embedded blank */
static char *
glue_strings(char *a, char *b)
{
    char *result;

    result = (char *) xmalloc(strlen(a) + strlen(b) + 2);
    strcpy(result, a);
    strcat(result, b);
    return result;
}
#endif

/* Append two declaration specifier parts together.
 */
void
join_decl_specs(DeclSpec * result, DeclSpec * a, DeclSpec * b)
{
    result->text = concat_string(a->text, b->text);
    result->flags = a->flags | b->flags;
    result->begin = a->begin;

    /* don't free the old text value - this is only used in one case where
     * the 'a' parameter holds the same data as 'result' did.
     */
}

/* Output an error message if the declaration specifier is an untagged
 * struct, union or enum.
 */
void
check_untagged(DeclSpec * decl_spec)
{
    if (strstr(decl_spec->text, "struct {}") != NULL) {
	put_error();
	fputs("untagged struct declaration\n", stderr);
    } else if (strstr(decl_spec->text, "union {}") != NULL) {
	put_error();
	fputs("untagged union declaration\n", stderr);
    } else if (strstr(decl_spec->text, "enum {}") != NULL) {
	put_error();
	fputs("untagged enum declaration\n", stderr);
    }
}

/* Allocate and initialize a declarator.
 */
Declarator *
new_declarator(const char *text, const char *name, long offset)
{
    Declarator *d;

    d = NEW(Declarator);
    d->text = xstrdup(text);
    d->name = xstrdup(name);
    d->begin = offset;
    d->begin_comment = d->end_comment = 0;
    d->func_def = FUNC_NONE;
    new_ident_list(&d->params);
    d->head = d;
    d->func_stack = NULL;
    d->pointer = FALSE;
    return d;
}

/* Free storage used by a declarator.
 */
void
free_declarator(Declarator * d)
{
    free(d->text);
    free(d->name);
    free_param_list(&d->params);
    if (d->func_stack != NULL)
	free_declarator(d->func_stack);
    free(d);
}

/* Initialize a declarator list and add the given declarator to it.
 */
void
new_decl_list(DeclaratorList * decl_list, Declarator * declarator)
{
    decl_list->first = decl_list->last = declarator;
    declarator->next = NULL;
}

/* Free storage used by the declarators in the declarator list.
 */
void
free_decl_list(DeclaratorList * decl_list)
{
    Declarator *d, *next;

    d = decl_list->first;
    while (d != NULL) {
	next = d->next;
	free_declarator(d);
	d = next;
    }
}

/* Add the declarator to the declarator list.
 */
void
add_decl_list(DeclaratorList * to, DeclaratorList * from, Declarator * declarator)
{
    to->first = from->first;
    from->last->next = declarator;
    to->last = declarator;
    to->last->next = NULL;
}

/* Create a new parameter structure.
 */
Parameter *
new_parameter(DeclSpec * decl_spec, Declarator * declarator)
{
    Parameter *param;
    param = NEW(Parameter);

    if (decl_spec == NULL) {
	new_decl_spec(&param->decl_spec, "", 0L, DS_NONE);
    } else {
	param->decl_spec = *decl_spec;
    }

    if (declarator == NULL) {
	declarator = new_declarator("", "", 0L);
    }
    param->declarator = declarator;

    param->comment = NULL;
    return param;
}

/* Free the storage used by the parameter.
 */
void
free_parameter(Parameter * param)
{
    free_decl_spec(&param->decl_spec);
    free_declarator(param->declarator);
    if (param->comment != NULL)
	free(param->comment);
    free(param);
}

/* Return TRUE if the parameter is void.
 */
static boolean
is_void_parameter(Parameter * p)
{
    return (boolean) ((p == NULL) ||
		      (strcmp(p->decl_spec.text, "void") == 0 &&
		       p->declarator->text[0] == '\0'));
}

/* Initialize a list of function parameters.
 */
void
new_param_list(ParameterList * param_list, Parameter * param)
{
    param_list->first = param_list->last = param;
    param->next = NULL;

    param_list->begin_comment = param_list->end_comment = 0;
    param_list->comment = NULL;
}

/* Free storage used by the elements in the function parameter list.
 */
void
free_param_list(ParameterList * param_list)
{
    Parameter *p, *next;

    p = param_list->first;
    while (p != NULL) {
	next = p->next;
	free_parameter(p);
	p = next;
    }

    if (param_list->comment != NULL)
	free(param_list->comment);
}

/* Add the parameter to the function parameter list.
 */
void
add_param_list(ParameterList * to, ParameterList * from, Parameter * param)
{
    to->first = from->first;
    from->last->next = param;
    to->last = param;
    param->next = NULL;
}

/* Initialize an empty list of function parameter names.
 */
void
new_ident_list(ParameterList * param_list)
{
    param_list->first = param_list->last = NULL;
    param_list->begin_comment = param_list->end_comment = 0;
    param_list->comment = NULL;
}

/* Add an item to the list of function parameter declarations but set only
 * the parameter name field.
 */
void
add_ident_list(ParameterList * to, ParameterList * from, const char *name)
{
    Parameter *p;
    Declarator *declarator;

    declarator = new_declarator(name, name, 0L);
    p = new_parameter((DeclSpec *) 0, declarator);

    to->first = from->first;
    if (to->first == NULL) {
	to->first = p;
    } else {
	from->last->next = p;
    }
    to->last = p;
    p->next = NULL;
}

/* Search the list of parameters for a matching parameter name.
 * Return a pointer to the matching parameter or NULL if not found.
 */
static Parameter *
search_parameter_list(ParameterList * params, char *name)
{
    Parameter *p;

    for (p = params->first; p != NULL; p = p->next) {
	if (strcmp(p->declarator->name, name) == 0)
	    return p;
    }
    return (Parameter *) NULL;
}

/* For each name in the declarator list <declarators>, set the declaration
 * specifier part of the parameter in <params> having the same name.
 * This is also where we promote formal parameters.  Parameters of type
 * "char", "unsigned char", "short", or "unsigned short" are promoted to
 * "int".  Parameters of type "float" are promoted to "double".
 */
void
set_param_types(ParameterList * params, DeclSpec * decl_spec, DeclaratorList
		* declarators)
{
    Declarator *d;
    Parameter *p;
    const char *decl_spec_text;

    for (d = declarators->first; d != NULL; d = d->next) {
	/* Search the parameter list for a matching name. */
	if ((p = search_parameter_list(params, d->name)) == NULL) {
	    put_error();
	    fprintf(stderr, "declared argument \"%s\" is missing\n", d->name);
	} else {
	    decl_spec_text = decl_spec->text;
	    if (promote_param && strcmp(d->text, d->name) == 0) {
		if (decl_spec->flags & (DS_CHAR | DS_SHORT))
		    decl_spec_text = "int";
		else if (decl_spec->flags & DS_FLOAT)
		    decl_spec_text = "double";
	    }
	    free(p->decl_spec.text);
	    p->decl_spec.text = xstrdup(decl_spec_text);

	    free_declarator(p->declarator);
	    p->declarator = d;
	}
    }
}

/* Output a function parameter.
 */
static int
put_parameter(FILE *outf,
	      Parameter * p,
#if OPT_LINTLIBRARY
	      int name_only,	/* nonzero if we only show the parameter name */
	      int count,	/* index in parameter list if we haven't names */
#endif
	      int commented)	/* comment-delimiters already from higher level */
{
    const char *s2;
    char *s;
    char gap = ' ';

#if OPT_LINTLIBRARY
    if (name_only) {
	s = p->declarator->name;
	if (lintLibrary()) {
	    while (*s == '*')
		s++;
	    if (*s == '\0' && !is_void_parameter(p))
		s = supply_parm(count);
	}
	put_string(outf, s);	/* ... remainder of p->declarator.name */
	return (TRUE);
    }
#endif

    s2 = p->decl_spec.text;
#if OPT_LINTLIBRARY
    if (lintLibrary()) {
	if (is_void_parameter(p))
	    return (FALSE);
	indent(outf);
	if (knrLintLibrary() && !*s2)
	    s2 = "int";
	if (strlen(s2) < 8)
	    gap = '\t';
    }
#endif

    put_string(outf, s2);

#if OPT_LINTLIBRARY
    if (lintLibrary()) {
	char *t, *u;
	s = p->declarator->text;
	while (*s == '*')
	    s++;
	if (*s == '\0') {
	    u = p->declarator->text;
	    p->declarator->text = glue_strings(u, supply_parm(count));
	    free(u);
	} else if (p->declarator->name[0] == '\0') {
	    if ((t = strstr(s, "%s")) != 0) {
		int parenthesized = FALSE;
		Declarator *q;

		free(p->declarator->name);
		p->declarator->name = xstrdup(supply_parm(count));

		for (q = p->declarator; q != 0; q = q->func_stack) {
		    if (q->func_def == FUNC_NONE) {
			if (!strcmp(q->text, "(*)")) {
			    char temp[20];
			    sprintf(temp, "(*%s)", p->declarator->name);
			    free(q->text);
			    q->text = xstrdup(temp);
			    parenthesized = TRUE;
			} else {
			    free(q->text);
			    q->text = xstrdup(p->declarator->name);
			}
			break;
		    }
		}
		if (!parenthesized) {
		    if (strchr(t, PAREN_L) != 0) {	/* e.g., "*%s()" */
			t = p->declarator->text;
			u = (char *) xmalloc(strlen(t) + 3);
			(void) sprintf(u, "(%s)", t);
			p->declarator->text = u;
			free(t);
		    }
		}
	    } else {		/* e.g., s is "[20]" for "char [20]" parameter */
		/* ...or something like "* const *" */
		while (*s != '\0' && *s != SQUARE_L)
		    s++;
		u = xstrdup(s);	/* the "[20]" */
		*s = '\0';
		if (s != p->declarator->text) {
		    s = glue_strings(p->declarator->text, supply_parm(count));
		} else {
		    s = xstrdup(supply_parm(count));
		}

		t = p->declarator->text;
		p->declarator->text = glue_strings(s, u);
		free(u);
		free(s);
		free(t);
	    }
	}
    }
#endif

    if (p->declarator->text[0] != '\0') {
	if (strcmp(p->declarator->text, ELLIPSIS) == 0) {
	    put_string(outf, ELLIPSIS);
	} else {
	    if (proto_style != PROTO_ABSTRACT || proto_comments
		|| where != FUNC_PROTO
		|| strcmp(p->declarator->text, p->declarator->name) != 0)
		put_char(outf, gap);
	    put_declarator(outf, p->declarator, commented);
	}
    }
    return (TRUE);
}

/* Output a parameter list.
 */
static void
put_param_list(FILE *outf, Declarator * declarator, int commented)
{
#if OPT_LINTLIBRARY
    int count = 0;
#endif
    Parameter *p;
    int f;
    int hide_it = (where == FUNC_PROTO) && (proto_style == PROTO_TRADITIONAL);
    int do_cmt = proto_comments && hide_it;

    p = declarator->params.first;
    if (hide_it && !do_cmt) {
	;
    } else if (is_void_parameter(p)) {
	if (do_cmt) {
	    if (!commented)
		put_string(outf, COMMENT_BEGIN);
	    put_string(outf, "void");
	    if (!commented)
		put_string(outf, COMMENT_END);
	} else if (!hide_it)
#if OPT_LINTLIBRARY
	    if (!knrLintLibrary())
#endif
		put_string(outf, "void");
    } else {
	f = (declarator == func_declarator) ? format : FMT_OTHER;

#if OPT_LINTLIBRARY
	if (where == FUNC_PROTO
	    && knrLintLibrary()
	    && (func_declarator != declarator)) {
	    do_cmt = TRUE;	/* patch: shouldn't have gotten here at all */
	}
#endif
	if (where == FUNC_DEF && declarator->params.comment != NULL)
	    put_string(outf, declarator->params.comment);
	else if (do_cmt && !commented)
	    put_string(outf, COMMENT_BEGIN);

	put_string(outf, fmt[f].first_param_prefix);
	(void) putParameter(outf, p, knrLintLibrary(), ++count, commented);

	while (p->next != NULL) {
#if OPT_LINTLIBRARY
	    if (lint_ellipsis(p->next))
		break;
#endif
	    put_char(outf, ',');
	    if (where == FUNC_DEF && p->comment != NULL)
		put_string(outf, p->comment);

	    p = p->next;
	    put_string(outf, fmt[f].middle_param_prefix);
	    (void) putParameter(outf, p, knrLintLibrary(), ++count, commented);
	}
	if (where == FUNC_DEF && p->comment != NULL)
	    put_string(outf, p->comment);
	else if (do_cmt && !commented)
	    put_string(outf, COMMENT_END);

	put_string(outf, fmt[f].last_param_suffix);
    }
}

/* Output function parameters.
 */
static void
put_parameters(FILE *outf, Declarator * declarator, int commented)
{
    Parameter *p;

    nestedParams++;
    if (where == FUNC_DEF && func_style == FUNC_TRADITIONAL) {

	/* Output parameter name list for traditional function definition. */
	p = declarator->params.first;

	/* Output parameter name list only for head function declarator. */
	if (!is_void_parameter(p) && declarator == func_declarator) {
	    put_string(outf, fmt[format].first_param_prefix);
	    put_string(outf, p->declarator->name);
	    p = p->next;
	    while (p != NULL && strcmp(p->declarator->text, ELLIPSIS) != 0) {
		put_char(outf, ',');
		put_string(outf, fmt[format].middle_param_prefix);
		put_string(outf, p->declarator->name);
		p = p->next;
	    }
	    put_string(outf, fmt[format].last_param_suffix);
	}
    } else {

	/* Output parameter type list. */
	if (where == FUNC_PROTO && proto_style == PROTO_TRADITIONAL &&
	    declarator == func_declarator) {
	    if (proto_comments) {
		put_string(outf, COMMENT_BEGIN);
		put_param_list(outf, declarator, TRUE);
		put_string(outf, COMMENT_END);
	    }
	} else if (func_style != FUNC_NONE || proto_style != PROTO_NONE) {
#if OPT_LINTLIBRARY
	    if (!knrLintLibrary() || nestedParams <= 1)
#endif
		put_param_list(outf, declarator, commented);
	}
    }
    nestedParams--;
}

/* Output a function declarator.
 */
static void
put_func_declarator(FILE *outf, Declarator * declarator, int commented)
{
    char *s, *t, *decl_text;
    int f;
    int saveNest = nestedParams;

    /* Output declarator text before function declarator place holder. */
    if ((s = strstr(declarator->text, "%s")) == NULL)
	return;
    *s = '\0';
    put_string(outf, declarator->text);

    /* Substitute place holder with function declarator. */
    if (declarator->func_stack->func_def == FUNC_NONE) {

	decl_text = declarator->func_stack->text;
	if (declarator->name[0] == '\0') {
	    put_string(outf, decl_text);
	} else {
	    int star;

	    /* Output the declarator text before the declarator name. */
	    if ((t = strstr(decl_text, declarator->name)) == NULL)
		return;
	    *t = '\0';
	    star = ((t != decl_text) && (t[-1] == '*'));
	    put_string(outf, decl_text);
	    *t = declarator->name[0];

	    /* Output the declarator prefix before the name. */
	    f = (declarator == func_declarator) ? format : FMT_OTHER;
	    if (strcmp(fmt[f].declarator_prefix, " ") != 0)
		put_string(outf, fmt[f].declarator_prefix);

	    /* Output the declarator name. */
	    if (where == FUNC_PROTO && proto_style == PROTO_ABSTRACT &&
		declarator != func_declarator) {
		if (proto_comments) {
		    if (star)
			put_char(outf, ' ');
		    put_string(outf, COMMENT_BEGIN);
		    put_string(outf, declarator->name);
		    put_string(outf, COMMENT_END);
		}
	    } else {
		put_string(outf, declarator->name);
	    }

	    /* Output the remaining declarator text. */
	    put_string(outf, t + strlen(declarator->name));

	    /* Output the declarator suffix. */
	    put_string(outf, fmt[f].declarator_suffix);
	}
    } else {
	put_func_declarator(outf, declarator->func_stack, commented);
	nestedParams = 2;	/* e.g., "void (*signal(p1, p2))()" */
    }
    *s = '%';
    s += 2;

    /* Output declarator text up to but before parameters place holder. */
    if ((t = strstr(s, "()")) == NULL)
	return;
    *t = '\0';
    put_string(outf, s);

    if (where == FUNC_PROTO
	&& (func_declarator == declarator
	    || func_declarator == declarator->head)
	&& proto_macro) {
	fprintf(outf, " %s(", macro_name);
    }

    /* Substitute place holder with function parameters. */
    put_char(outf, *t++ = PAREN_L);
    put_parameters(outf, declarator, commented);
    put_string(outf, t);

    if (where == FUNC_PROTO
	&& (func_declarator == declarator
	    || func_declarator == declarator->head)
	&& proto_macro) {
	put_char(outf, PAREN_R);
    }
    nestedParams = saveNest;
}

/* Output a declarator.
 */
static void
put_declarator(FILE *outf, Declarator * declarator, int commented)
{
    char *s;

    if (declarator->func_def == FUNC_NONE) {
	if (where == FUNC_PROTO && proto_style == PROTO_ABSTRACT &&
	    declarator->name[0] != '\0') {
	    if ((s = strstr(declarator->text, declarator->name)) == NULL)
		return;
	    *s = '\0';
	    if (proto_comments) {
		fprintf(outf, "%s%s%s%s%s", declarator->text,
			COMMENT_BEGIN, declarator->name, COMMENT_END,
			s + strlen(declarator->name));
	    } else {
		fprintf(outf, "%s%s", declarator->text,
			s + strlen(declarator->name));
	    }
	    *s = declarator->name[0];
	} else {
	    put_string(outf, declarator->text);
	}
    } else {
	put_func_declarator(outf, declarator, commented);
    }
}

/* Output a declaration specifier for an external declaration.
 */
static void
put_decl_spec(FILE *outf, DeclSpec * decl_spec)
{
    /* An "extern func()" is legal, but we want to be explicit for lint libs */
#if OPT_LINTLIBRARY
    if (decl_spec->text[0] == '\0') {
	free(decl_spec->text);
	decl_spec->text = xstrdup("int");
    }
#endif
    if (extern_out && !(decl_spec->flags & DS_STATIC) &&
	strkey(decl_spec->text, "extern") == NULL)
	put_padded(outf, "extern");
    put_padded(outf, decl_spec->text);
}

/* Output the list of parameters in K&R style, for lint-library
 */
#if OPT_LINTLIBRARY
static void
put_llib_params(Declarator * declarator, int commented)
{
    Parameter *p;
    int count = 0;

    nestedParams++;
    p = (declarator->func_stack->func_def != FUNC_NONE)
	? declarator->func_stack->params.first
	: declarator->params.first;

    while (p != 0) {
	if (lint_ellipsis(p))
	    break;
	if (putParameter(stdout, p, FALSE, ++count, commented))
	    putchar(';');
	p = p->next;
    }
    nestedParams--;
}
#endif

/* Generate variable declarations.
 */
void
gen_declarations(DeclSpec * decl_spec,	/* declaration specifier */
		 DeclaratorList * decl_list)	/* list of declared variables */
{
    Declarator *d;
    int commented = FALSE;
    int saveNest = nestedParams;

#if OPT_LINTLIBRARY
    boolean defines = (boolean) (strchr(decl_spec->text, CURL_L) != 0);
    int is_func;

    /* special treatment for -l, -T options */
    if ((!variables_out && types_out && defines) || (decl_list == 0)) {
	strcut(decl_spec->text, "static");
	strcut(decl_spec->text, "extern");
	fmt_library((decl_list == 0) ? 1 : 2);
	if (decl_spec->text[0] != '\0') {
	    put_string(stdout, decl_spec->text);
	    put_string(stdout, ";\n");
	}
	return;
    }
#endif

    if (!variables_out || (decl_spec->flags & (DS_EXTERN | DS_INLINE | DS_JUNK))) {
#if OPT_LINTLIBRARY
	if (in_include >= extern_in)	/* -x option not set? */
#endif
	    return;
#if OPT_LINTLIBRARY
	strcut(decl_spec->text, "extern");
#endif
    }
    if (scope_out == SCOPE_EXTERN && (decl_spec->flags & DS_STATIC))
	return;
    if (scope_out == SCOPE_STATIC && !(decl_spec->flags & DS_STATIC))
	return;
    if (!inline_out && (decl_spec->flags & DS_INLINE))
	return;

    check_untagged(decl_spec);
    func_declarator = NULL;
    where = FUNC_OTHER;
    format = FMT_OTHER;
    nestedParams = 0;

    for (d = decl_list->first; d != NULL; d = d->next) {
	if (d->func_def == FUNC_NONE
	    || d->head->func_stack->pointer
#if OPT_LINTLIBRARY
	    || (in_include < extern_in)
#endif
	    ) {
#if OPT_LINTLIBRARY
	    if (already_declared(d->name)) {
		flush_varargs();
		continue;
	    }

	    /*
	     * Try to distinguish function declarations from function pointer
	     * declarations, so that we don't unintentionally emit lint-library
	     * arguments for function pointers.
	     */
	    is_func = is_actual_func(d);

	    if (is_func) {
		ellipsis_varargs(d);
	    } else {
		nestedParams = 2;	/* disable params altogether */
		if (types_out)
		    fmt_library(2);
	    }
	    if (lint_shadowed && lintLibrary())
		printf("#undef %s\n", d->name);
#endif
	    put_string(stdout, fmt[FMT_PROTO].decl_spec_prefix);
	    put_decl_spec(stdout, decl_spec);
	    put_declarator(stdout, d, commented);
#if OPT_LINTLIBRARY
	    if (knrLintLibrary() && is_func)
		put_llib_params(d, commented);
#endif
	    put_body(stdout, decl_spec, d);
	    nestedParams = saveNest;
	}
#if OPT_LINTLIBRARY
	flush_varargs();
#endif
    }
    exitlike_func = FALSE;
}

/* Return TRUE if the function uses varargs.
 */
static int
uses_varargs(Declarator * declarator)
{
    Parameter *p;

    return (p = declarator->params.first) != NULL
	&& (p->next == NULL)
	&& (!strcmp(p->declarator->name, "va_alist"));
}

/* If the parameter list is empty, then replace it with "void".
 */
static void
check_void_param(Declarator * declarator)
{
    DeclSpec decl_spec;
    Parameter *p;

    if (declarator->params.first == NULL) {
	new_decl_spec(&decl_spec, "void", 0L, DS_NONE);
	p = new_parameter(&decl_spec, (Declarator *) 0);
	new_param_list(&declarator->params, p);
    }
}

/* If a parameter name appears in the parameter list of a traditional style
 * function definition but is not declared in the parameter declarations,
 * then assign it the default type "int".
 */
static void
set_param_decl_spec(Declarator * declarator)
{
    Parameter *p;

    for (p = declarator->params.first; p != NULL; p = p->next) {
	if (p->decl_spec.text[0] == '\0' &&
	    strcmp(p->declarator->text, ELLIPSIS) != 0) {
	    free(p->decl_spec.text);
	    p->decl_spec.text = xstrdup("int");
	}
    }
}

/* Generate a function prototype.
 */
void
gen_prototype(DeclSpec * decl_spec, Declarator * declarator)
{
    Parameter *p;
    int commented = FALSE;

    if (proto_style == PROTO_NONE || (decl_spec->flags & DS_JUNK))
	return;
    if (scope_out == SCOPE_EXTERN && (decl_spec->flags & DS_STATIC))
	return;
    if (scope_out == SCOPE_STATIC && !(decl_spec->flags & DS_STATIC))
	return;
    if (!inline_out && (decl_spec->flags & DS_INLINE))
	return;
#if OPT_LINTLIBRARY
    if ((decl_spec->flags & DS_INLINE)) {
	if (in_include > extern_in)	/* -x option not set? */
	    return;
    }
#endif

    /*
     * Trim pathological keywords (which are legal, but unnecessary) from the
     * function and its parameters.
     */
    strcut(decl_spec->text, "extern");
    for (p = declarator->params.first; p != NULL; p = p->next) {
	strcut(p->decl_spec.text, "extern");
	strcut(p->decl_spec.text, "auto");
    }

#if OPT_LINTLIBRARY
    if (lintLibrary())
	ellipsis_varargs(declarator);
    else if (types_out)
	fmt_library(0);
#endif

    func_declarator = declarator->head;
    if (uses_varargs(func_declarator)) {
	/* Generate a prototype for a function that uses varargs by replacing
	 * the "va_alist" parameter with an empty parameter list.
	 */
	free_param_list(&func_declarator->params);
	func_declarator->params.first = NULL;
    }

    check_void_param(func_declarator);
    set_param_decl_spec(func_declarator);

    where = FUNC_PROTO;
    format = FMT_PROTO;
    nestedParams = 0;

#if OPT_LINTLIBRARY
    if (lint_shadowed && lintLibrary())
	printf("#undef %s\n", declarator->name);
#endif
    put_string(stdout, fmt[format].decl_spec_prefix);
    put_decl_spec(stdout, decl_spec);
    put_func_declarator(stdout, declarator, commented);
#if OPT_LINTLIBRARY
    if (knrLintLibrary())
	put_llib_params(declarator, commented);
#endif
    put_body(stdout, decl_spec, declarator);
}

/* Generate a declarator for a function pointer declarator or prototype.
 */
void
gen_func_declarator(Declarator * declarator)
{
    /* Go to the beginning of the function declarator in the temporary
     * file and overwrite it with the converted declarator.
     */
    fseek(cur_tmp_file(), declarator->begin, 0);
    func_declarator = NULL;

    where = FUNC_DEF;
    format = FMT_FUNC;
    nestedParams = 0;

    put_func_declarator(cur_tmp_file(), declarator, FALSE);
    cur_file_changed();
}

/* Output parameter declarations for old style function definition.
 */
static void
put_param_decl(Declarator * declarator, int commented)
{
#if OPT_LINTLIBRARY
    int count = 0;
#endif
    Parameter *p;

    p = declarator->params.first;
    if (!is_void_parameter(p)) {
	fputc('\n', cur_tmp_file());
	(void) putParameter(cur_tmp_file(), p, knrLintLibrary(), ++count, commented);
	fputc(';', cur_tmp_file());
	if (p->comment != 0)
	    fputs(p->comment, cur_tmp_file());
	p = p->next;
	while (p != NULL && strcmp(p->declarator->text, ELLIPSIS) != 0) {
	    fputc('\n', cur_tmp_file());
	    (void) putParameter(cur_tmp_file(), p, knrLintLibrary(),
				++count, commented);
	    fputc(';', cur_tmp_file());
	    if (p->comment != 0)
		fputs(p->comment, cur_tmp_file());
	    p = p->next;
	}
    }
}

/* Generate a function definition head.
 */
void
gen_func_definition(DeclSpec * decl_spec, Declarator * declarator)
{
    Parameter *p;
    ParameterList *params;
    char *comment = 0;
    int n;
    size_t comment_len;
    long diff;

    /* Do nothing if the function is already defined in the desired style
     * or if the function uses varargs.
     */
    func_declarator = declarator->head;
    if (func_declarator->func_def == func_style ||
	uses_varargs(func_declarator))
	return;

    /* Save the text between the function head and the function body.
     * Read the temporary file from after the last ) or ; to the
     * end of the file.
     */
    if ((diff = (ftell(cur_tmp_file()) - cur_begin_comment())) > 0) {
	comment_len = (size_t) diff;
	*(comment = (char *) xmalloc(comment_len)) = '\0';
	fseek(cur_tmp_file(), cur_begin_comment(), 0);
	fread(comment, sizeof(char), comment_len, cur_tmp_file());
    } else {
	comment_len = 0;
    }

    format = FMT_FUNC;
    nestedParams = 0;

    if (func_declarator->func_def == FUNC_TRADITIONAL
	|| func_declarator->func_def == FUNC_BOTH) {
	/* Save the text before the parameter declarations. */
	params = &func_declarator->params;
	n = (int) (params->end_comment - params->begin_comment);
	if (n > 0) {
	    *(params->comment = (char *) xmalloc((size_t) (n + 1))) = '\0';
	    fseek(cur_tmp_file(), params->begin_comment, 0);
	    fread(params->comment, sizeof(char), (size_t) n, cur_tmp_file());
	    params->comment[n] = '\0';
	    format = FMT_FUNC_COMMENT;
	}

	/* Get the parameter comments. */
	for (p = func_declarator->params.first; p != NULL; p = p->next) {
	    n = (int) (p->declarator->end_comment - p->declarator->begin_comment);
	    if (n > 0) {
		*(p->comment = (char *) xmalloc((size_t) n + 1)) = '\0';
		fseek(cur_tmp_file(), p->declarator->begin_comment, 0);
		fread(p->comment, sizeof(char), (size_t) n, cur_tmp_file());
		p->comment[n] = '\0';
		format = FMT_FUNC_COMMENT;
	    }
	}
    }

    check_void_param(func_declarator);
    set_param_decl_spec(func_declarator);

    /* Go to the beginning of the function head in the temporary file
     * and overwrite it with the converted function head.
     */
    where = FUNC_DEF;
    fseek(cur_tmp_file(), decl_spec->begin, 0);

    if (func_style == FUNC_BOTH) {
	char *cur_func;
	size_t func_len;

	/* Save the current function definition head. */
	if ((diff = (cur_begin_comment() - decl_spec->begin)) > 0) {
	    func_len = (size_t) diff;
	    cur_func = (char *) xmalloc(func_len);
	    fread(cur_func, sizeof(char), func_len, cur_tmp_file());
	} else {
	    cur_func = 0;
	    func_len = 0;
	}

	fseek(cur_tmp_file(), decl_spec->begin, 0);
	fprintf(cur_tmp_file(), "%s\n\n", func_directive);

	/* Output new style function definition head. */
	if (func_declarator->func_def == FUNC_ANSI) {
	    if (cur_func != 0)
		fwrite(cur_func, sizeof(char), func_len, cur_tmp_file());
	} else {
	    fputs(fmt[format].decl_spec_prefix, cur_tmp_file());
	    fputs(decl_spec->text, cur_tmp_file());
	    fputc(' ', cur_tmp_file());

	    func_style = FUNC_ANSI;
	    put_func_declarator(cur_tmp_file(), declarator, FALSE);
	}
	fputs("\n#else\n\n", cur_tmp_file());

	/* Output old style function definition head. */
	if (func_declarator->func_def == FUNC_TRADITIONAL
	    || func_declarator->func_def == FUNC_BOTH) {
	    if (cur_func != 0)
		fwrite(cur_func, sizeof(char), func_len, cur_tmp_file());
	} else {
	    fputs(fmt[format].decl_spec_prefix, cur_tmp_file());
	    fputs(decl_spec->text, cur_tmp_file());
	    fputc(' ', cur_tmp_file());

	    format = FMT_FUNC;
	    func_style = FUNC_TRADITIONAL;
	    put_func_declarator(cur_tmp_file(), declarator, FALSE);
	    put_param_decl(func_declarator, FALSE);
	}

	fputs("\n#endif", cur_tmp_file());
	if (comment != 0 && *comment != '\n')
	    fputc('\n', cur_tmp_file());
	func_style = FUNC_BOTH;

	if (cur_func != 0)
	    free(cur_func);

    } else {
	/* Output declarator specifiers. */
	fputs(fmt[format].decl_spec_prefix, cur_tmp_file());
	fputs(decl_spec->text, cur_tmp_file());
	fputc(' ', cur_tmp_file());

	/* Output function declarator. */
	put_func_declarator(cur_tmp_file(), declarator, FALSE);
	if (func_style == FUNC_TRADITIONAL)
	    put_param_decl(func_declarator, FALSE);
    }

    /* Output text between function head and body. */
    if (comment != 0) {
	fwrite(comment, sizeof(char), comment_len, cur_tmp_file());
	free(comment);
    }

    cur_file_changed();
}
