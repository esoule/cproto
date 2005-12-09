# $Id: borland.mak,v 4.2 2005/12/08 20:34:07 tom Exp $
#
# Borland C++ makefile for C prototype generator

# Define MSDOS for MS-DOS compatibility.
# Define TURBO_CPP to pipe the input through the Turbo C preprocessor.
DEFINES = -DMSDOS -DTURBO_CPP

LEX = lex
YACC = yacc
CC = bcc
CFLAGS = -mc $(DEFINES) -w-pin -w-pro
LIBS = \bc\lib\wildargs.obj

DIST1 = README CHANGES cproto.man
DIST2 = cproto.1 borland.mak micrsoft.mak makefile.in lex.l grammar.y
DIST3 = system.h cproto.h patchlev.h semantic.h symbol.h
DIST4 = cproto.c popen.c lintlibs.c semantic.c strkey.c symbol.c

OBJECTS = cproto.obj lintlibs.obj getopt.obj popen.obj semantic.obj strkey.obj symbol.obj y_tab.obj

all: cproto.exe

cproto.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -e$*.exe $(OBJECTS) $(LIBS)

y_tab.obj: y_tab.c lex_yy.c system.h cproto.h symbol.h semantic.h
	$(CC) $(CFLAGS) -c $*.c

y_tab.c: grammar.y
	$(YACC) grammar.y

lex_yy.c: lex.l
	$(LEX) lex.l

cproto.man: cproto.1
	cawf -man $*.1 | bsfilt - >$*.man

clean:
	erase *.obj
	erase *.bak
	erase *.log
	erase lex_yy.c
	erase y_tab.c
	erase cproto.exe

ci:
	ci -u3 $(DIST2) $(DIST3)
	ci -u3 $(DIST4)

shar:
	shar $(DIST1) $(DIST2) >cproto.sh1
	shar $(DIST3) $(DIST4) >cproto.sh2

# DO NOT DELETE THIS LINE -- make depend depends on it.

cproto.obj: system.h cproto.h symbol.h
lintlibs.obj: system.h cproto.h symbol.h semantic.h
popen.obj: system.h cproto.h
semantic.obj: system.h cproto.h symbol.h semantic.h
strkey.obj: cproto.h system.h
symbol.obj: system.h cproto.h symbol.h
