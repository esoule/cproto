/* $Id: symbol.c,v 4.9 2011/01/02 19:17:15 tom Exp $
 *
 * Implements a symbol table abstract data type.
 */
#include <symbol.h>

/* Create a symbol table.
 * Return a pointer to the symbol table or NULL if an error occurs.
 */
SymbolTable *
new_symbol_table(void)
{
    SymbolTable *symtab;
    int i;

    if ((symtab = NEW(SymbolTable)) != NULL) {
	for (i = 0; i < SYM_MAX_HASH; ++i)
	    symtab->bucket[i] = NULL;
    }
    return symtab;
}

/* Free the memory allocated to the symbol table.
 */
void
free_symbol_table(SymbolTable * symtab)
{
    int i;
    Symbol *sym, *next;

    for (i = 0; i < SYM_MAX_HASH; ++i) {
	sym = symtab->bucket[i];
	while (sym != NULL) {
	    next = sym->next;
	    if (sym->name != 0)
		free(sym->name);
	    if (sym->value != 0)
		free(sym->value);
	    free(sym);
	    sym = next;
	}
    }
    free(symtab);
}

/* This is a simple hash function mapping a symbol name to a hash bucket. */

static unsigned
hash(const char *name)
{
    const char *s;
    unsigned h;

    h = 0;
    s = name;
    while (*s != '\0')
	h = (h << 1) ^ (unsigned char) (*s++);
    return h % SYM_MAX_HASH;
}

/* Search the list of symbols <list> for the symbol <name>.
 * Return a pointer to the symbol or NULL if not found.
 */
static Symbol *
search_symbol_list(Symbol * list, const char *name)
{
    Symbol *sym;

    for (sym = list; sym != NULL; sym = sym->next) {
	if (strcmp(sym->name, name) == 0)
	    return sym;
    }
    return NULL;
}

/* Look for symbol <name> in symbol table <symtab>.
 * Return a pointer to the symbol or NULL if not found.
 */
Symbol *
find_symbol(SymbolTable * symtab, const char *name)
{
    return search_symbol_list(symtab->bucket[hash(name)], name);
}

/* If the symbol <name> does not already exist in symbol table <symtab>,
 * then add the symbol to the symbol table.
 * Return a pointer to the symbol.
 */
Symbol *
new_symbol(
	      SymbolTable * symtab,	/* symbol table */
	      const char *name,	/* symbol name */
	      const char *value,	/* symbol value */
	      int flags)	/* symbol attributes */
{
    Symbol *sym;
    unsigned i;

    if ((sym = find_symbol(symtab, name)) == NULL) {
	sym = NEW(Symbol);
	sym->name = xstrdup(name);
	i = hash(name);
	sym->next = symtab->bucket[i];
	symtab->bucket[i] = sym;
    } else {
	free(sym->value);
    }
    sym->value = (value != NULL) ? xstrdup(value) : NULL;
    sym->flags = (short) flags;
    return sym;
}
