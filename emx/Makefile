# $Id: Makefile,v 4.3 1998/01/22 19:49:36 cthuang Exp $
#
# EMX makefile for C prototype generator

PROGRAM = cproto
DEFINES =
INCLUDES = -I..

LEX = lex
YACC = yacc
CC = gcc
CFLAGS = $(DEFINES) $(INCLUDES)
LIBS =

VPATH = ..

O = .o
OBJECTS = \
	cproto$(O) \
	lintlibs$(O) \
	semantic$(O) \
	strkey$(O) \
	symbol$(O) \
	y_tab$(O)

all: cproto.exe

cproto.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)

cproto.man: cproto.1
	cawf -man $*.1 | bsfilt - >$*.man

clean:
	-del $(PROGRAM).exe
	-del *$(O)
	-del *.bak
	-del *.log

# DO NOT DELETE THIS LINE -- make depend depends on it.

cproto.o: system.h cproto.h symbol.h
lintlibs.o: system.h cproto.h symbol.h semantic.h
semantic.o: system.h cproto.h symbol.h semantic.h
strkey.o: cproto.h system.h
symbol.o: system.h cproto.h symbol.h
y_tab.o: lex_yy.c system.h cproto.h symbol.h semantic.h
