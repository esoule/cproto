/* $Id: semantic.h,v 4.6 2011/01/02 19:03:03 tom Exp $
 *
 * Declarations of semantic action routines
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <cproto.h>

extern void new_decl_spec (
    DeclSpec *decl_spec, const char *text, long offset, int flags);
extern void free_decl_spec (
    DeclSpec *decl_spec);
extern void join_decl_specs (
    DeclSpec *result, DeclSpec *a, DeclSpec *b);
extern void check_untagged (
    DeclSpec *decl_spec);
extern Declarator *new_declarator (
    const char *text, const char *name, long offset);
extern void free_declarator (
    Declarator *d);
extern void new_decl_list (
    DeclaratorList *decl_list, Declarator *declarator);
extern void free_decl_list (
    DeclaratorList *decl_list);
extern void add_decl_list (
    DeclaratorList *to, DeclaratorList *from, Declarator *declarator);
extern Parameter *new_parameter (
    DeclSpec *decl_spec, Declarator *declarator);
extern void free_parameter (
    Parameter *param);
extern void new_param_list (
    ParameterList *param_list, Parameter *param);
extern void free_param_list (
    ParameterList *param_list);
extern void add_param_list (
    ParameterList *to, ParameterList *from, Parameter *param);
extern void new_ident_list (
    ParameterList *param_list);
extern void add_ident_list (
    ParameterList *to, ParameterList *from, const char *name);
extern void set_param_types (
    ParameterList *params, DeclSpec *decl_spec, DeclaratorList *declarators);
extern void gen_declarations (
    DeclSpec *decl_spec, DeclaratorList *decl_list);
extern void gen_prototype (
    DeclSpec *decl_spec, Declarator *declarator);
extern void gen_func_declarator (
    Declarator *declarator);
extern void gen_func_definition (
    DeclSpec *decl_spec, Declarator *declarator);

#ifdef __cplusplus
}
#endif
