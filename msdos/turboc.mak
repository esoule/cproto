# $Id: turboc.mak,v 4.3 2005/12/08 20:34:07 tom Exp $
#
# Turbo C/C++ makefile for C prototype generator
# (adapted from PRJ2MAK output)
.AUTODEPEND 

# Define MSDOS for MS-DOS compatibility.
# Define TURBO_CPP to pipe the input through the Turbo C preprocessor.
DEFINES = MSDOS;TURBO_CPP

# There's no standard for the names produced by lex clones on MSDOS:
#	FLEX produces LEXYY.C
#	MKS LEX produces LEX_YY.C (I think)
#LEX = lex
#LEX_OUT=lex_yy
LEX = flex
LEX_OUT=lexyy

#YACC = yacc
YACC = byacc
#YACC = bison -o y_tab.c

#		*Translator Definitions*
CC = tcc +cproto.cfg
MODEL = l
TLIB = tlib
TLINK = tlink
LIBPATH = C:\TC\LIB
LIBS = $(LIBPATH)\wildargs.obj
INCLUDEPATH = .;\tc\include

DOC_FILES = \
	README \
	CHANGES \
	cproto.man \
	cproto.1

MAK_FILES = \
	makefile.in configure.in \
	turboc.mak \
	borland.mak \
	micrsoft.mak

DIST2 = lex.l grammar.y
DIST3 = system.h cproto.h patchlev.h semantic.h symbol.h
DIST4 = cproto.c popen.c lintlibs.c semantic.c strkey.c symbol.c

OBJECTS = \
	cproto.obj \
	lintlibs.obj \
	getopt.obj \
	popen.obj \
	semantic.obj \
	strkey.obj \
	symbol.obj \
	y_tab.obj

all: cproto.exe

y_tab.obj : cproto.cfg \
	y_tab.c $(LEX_OUT).c system.h cproto.h symbol.h semantic.h
	$(CC) -w-rch -c {$*.c }

y_tab.c : grammar.y
	$(YACC) grammar.y

$(LEX_OUT).c: lex.l
	$(LEX) lex.l

cproto.man: cproto.1
	cawf -man $*.1 | bsfilt - >$*.man

clean:
	erase *.$$$
	erase cproto.cfg
	erase *.out
	erase *.obj
	erase *.bak
	erase *.log
	erase $(LEX_OUT).c
	erase y_tab.c
	erase cproto.exe

#		*Implicit Rules*
.c.obj:
	$(CC) -c {$< }

.cpp.obj:
	$(CC) -c {$< }

#		*Explicit Rules*
cproto.exe: cproto.cfg $(OBJECTS)
	$(TLINK) /v/x/c/L$(LIBPATH) @&&|
c0$(MODEL).obj+
cproto.obj +
lintlibs.obj +
getopt.obj +
popen.obj +
semantic.obj +
strkey.obj +
symbol.obj +
y_tab.obj +
$(LIBS)
cproto
		# no map file
emu.lib+
math$(MODEL).lib+
c$(MODEL).lib
|

#		*Compiler Configuration File*
cproto.cfg: turboc.mak
	copy &&|
-m$(MODEL)
-v
-vi-
-w-ret
-w-nci
-w-inl
-wpin
-wamb
-wamp
-w-par
-wasm
-wcln
-w-cpt
-wdef
-w-dup
-w-pia
-wsig
-wnod
-w-ill
-w-sus
-wstv
-wucp
-wuse
-w-ext
-w-ias
-w-ibc
-w-pre
-w-nst
-I$(INCLUDEPATH)
-L$(LIBPATH)
-D$(DEFINES);STDC_HEADERS=1;USE_$(LEX);YY_USE_PROTOS
| cproto.cfg

# DO NOT DELETE THIS LINE -- make depend depends on it.

cproto.obj : cproto.cfg system.h cproto.h symbol.h
lintlibs.obj : cproto.cfg system.h cproto.h symbol.h semantic.h
popen.obj : cproto.cfg system.h cproto.h
semantic.obj : cproto.cfg system.h cproto.h symbol.h semantic.h
strkey.obj : cproto.cfg cproto.h system.h
symbol.obj : cproto.cfg system.h cproto.h symbol.h
