/* $Id: dump.h,v 4.5 1996/04/27 12:47:39 tom Exp $ */

#ifndef DUMP_H
#define DUMP_H

#include "cproto.h"

extern void dump_parameter(Parameter *p, int level);
extern void dump_param_list(ParameterList *p, int level);
extern void dump_declarator(Declarator *d, int level);
extern void dump_decl_spec(DeclSpec *d, int level);

#endif /* DUMP_H */
