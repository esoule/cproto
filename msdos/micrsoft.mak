# $Id: micrsoft.mak,v 4.2 2005/12/08 20:34:07 tom Exp $
#
# Microsoft C makefile for C prototype generator
# tested with:
#	nmake 1.11.
#	cl 6.00ax.

# Define MSDOS for MS-DOS compatibility.
# Define TURBO_CPP to pipe the input through the Turbo C preprocessor.
DEFINES = -DMSDOS -DSTDC_HEADERS

MSC_TOP = d:\msc60ax

# names from MKS tools (ran out of memory in lex; used unix output)
Y_TAB = ytab
LEX_YY = lex_yy

LEX = lex
YACC = yacc
CC = cl
LINK = link
CFLAGS = -EM -AL $(DEFINES)
LIBS = $(MSC_TOP)\lib\setargv.obj
LDFLAGS = /F 1000

DIST1 = README CHANGES cproto.man
DIST2 = cproto.1 borland.mak micrsoft.mak makefile.in lex.l grammar.y
DIST3 = system.h cproto.h patchlev.h semantic.h symbol.h
DIST4 = cproto.c lintlibs.c popen.c semantic.c strkey.c symbol.c

OBJECTS = cproto.obj lintlibs.obj getopt.obj semantic.obj strkey.obj symbol.obj $(Y_TAB).obj popen.obj

all: cproto.exe

cproto.exe: $(OBJECTS) micrsoft.lnk
	$(LINK) @micrsoft.lnk

micrsoft.lnk:
	for %%i in ( *.obj ) do echo %%i + >>$@
	echo >>$@
	echo cproto.exe >>$@
	echo >>$@
	echo $(LIBS); >>$@

$(Y_TAB).obj: $(Y_TAB).c $(LEX_YY).c system.h cproto.h symbol.h semantic.h
	$(CC) $(CFLAGS) -c $*.c

$(Y_TAB).c: grammar.y
	$(YACC) grammar.y

$(LEX_YY).c: lex.l
	$(LEX) lex.l

cproto.man: cproto.1
	cawf -man $*.1 | bsfilt - >$*.man

clean:
	erase *.obj
	erase *.bak
	erase *.log
	erase $(LEX_YY).c
	erase $(Y_TAB).c
	erase micrsoft.lnk
	erase cproto.exe

ci:
	ci -u3 $(DIST2) $(DIST3)
	ci -u3 $(DIST4)

shar:
	shar $(DIST1) $(DIST2) >cproto.sh1
	shar $(DIST3) $(DIST4) >cproto.sh2

# DO NOT DELETE THIS LINE -- make depend depends on it.

cproto.obj: system.h cproto.h
lintlibs.obj: system.h cproto.h symbol.h semantic.h
popen.obj: system.h cproto.h
semantic.obj: system.h cproto.h symbol.h semantic.h
strkey.obj: cproto.h system.h
symbol.obj: system.h cproto.h symbol.h
