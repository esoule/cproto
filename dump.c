/* $Id: dump.c,v 4.5 2011/01/02 19:17:15 tom Exp $
 *
 * useful dumps for cproto
 */
#include <trace.h>
#include <dump.h>

static char *whatFuncDefStyle(FuncDefStyle func_def);
static char *flagsDeclSpec(int flags);

#define PAD char pad[80]; sprintf(pad, "%-*s", level * 3, ".")

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG > 1
#define SHOW_CMTS(p) Trace p;
#else
#define SHOW_CMTS(p)
#endif

static char *
whatFuncDefStyle(FuncDefStyle func_def)		/* style of function definition */
{
    switch (func_def) {
    case FUNC_NONE:
	return "FUNC_NONE";
    case FUNC_TRADITIONAL:
	return "FUNC_TRADITIONAL";
    case FUNC_ANSI:
	return "FUNC_ANSI";
    case FUNC_BOTH:
	return "FUNC_BOTH";
    }
    return "?";
}

void
dump_parameter(Parameter * p, int level)
{
    dump_declarator(p->declarator, level + 1);
    dump_decl_spec(&(p->decl_spec), level + 1);
}

void
dump_param_list(ParameterList * p, int level)
{
    struct parameter *q;

    for (q = p->first; q != 0; q = q->next) {
	dump_parameter(q, level);
    }
}

void
dump_declarator(Declarator * d, int level)
{
    PAD;
    Trace("%sdeclarator %p\n", pad, d);
    Trace("%s   name /%s/\n", pad, d->name);
    Trace("%s   text /%s/\n", pad, d->text);
    SHOW_CMTS(("%s   begin %ld\n", pad, d->begin))
	SHOW_CMTS(("%s   begin_comment %ld\n", pad, d->begin_comment))
	SHOW_CMTS(("%s   end_comment %ld\n", pad, d->end_comment))
	Trace("%s   func_def %s\n", pad, whatFuncDefStyle(d->func_def));

#if DEBUG > 1
    if (d->func_def != FUNC_NONE) {
	Trace("%s >PARAMS of %p\n", pad, d);
	dump_param_list(&(d->params), level + 1);
    }
#endif
    Trace("%s   pointer %s\n", pad, d->pointer ? "YES" : "NO");

    if (d->head != 0 && d != d->head) {
	Trace("%s >HEAD of %p\n", pad, d);
	dump_declarator(d->head, level + 1);
    }
    if (d->func_stack != 0) {
	Trace("%s >FUNC_STACK of %p\n", pad, d);
	dump_declarator(d->func_stack, level + 1);
    }
    if (d->next != 0) {
	Trace("%s >NEXT of %p\n", pad, d);
	dump_declarator(d->next, level + 1);
    }
}

static char *
flagsDeclSpec(int flags)
{
    static char temp[100];
    static struct {
	int mask;
	char *text;
    } table[] = {
	{
	    DS_EXTERN, "extern"
	},
	{
	    DS_STATIC, "static"
	},
	{
	    DS_CHAR, "char"
	},
	{
	    DS_SHORT, "short"
	},
	{
	    DS_FLOAT, "float"
	},
	{
	    DS_JUNK, "junk"
	},
    };
    unsigned j;
    *temp = 0;
    for (j = 0; j < sizeof(table) / sizeof(table[0]); j++) {
	if (flags & table[j].mask) {
	    if (*temp)
		(void) strcat(temp, ",");
	    (void) strcat(temp, table[j].text);
	}
    }
    return temp;
}

void
dump_decl_spec(DeclSpec * d, int level)
{
    PAD;
    Trace("%sdecl_spec %p\n", pad, d);
    Trace("%s  flags %s\n", pad, flagsDeclSpec(d->flags));
    Trace("%s  text /%s/\n", pad, d->text);
    SHOW_CMTS(("%s  begin %ld\n", pad, d->begin))
}
