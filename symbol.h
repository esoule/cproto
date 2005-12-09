/* $Id: symbol.h,v 4.2 2004/03/24 19:56:11 tom Exp $
 *
 * A symbol table is a collection of string identifiers stored in a
 * hash table.
 */
#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct symbol {
    struct symbol *next;	/* next symbol in list */
    char *name; 		/* name of symbol */
    char *value;		/* value of symbol (for defines) */
    short flags;		/* symbol attributes */
} Symbol;

/* The hash table length should be a prime number. */
#define SYM_MAX_HASH 251

typedef struct symbol_table {
    Symbol *bucket[SYM_MAX_HASH];	/* hash buckets */
} SymbolTable;

extern SymbolTable *new_symbol_table	/* Create symbol table */
	(void);
extern void free_symbol_table		/* Destroy symbol table */
	(SymbolTable *s);
extern Symbol *find_symbol		/* Lookup symbol name */
	(SymbolTable *s, char *n);
extern Symbol *new_symbol		/* Define new symbol */
	(SymbolTable *s, char *n, char *v, int f);

#endif
