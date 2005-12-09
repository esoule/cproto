# $Id: watcom.mak,v 4.3 1998/01/22 19:49:44 cthuang Exp $
#
# Watcom makefile for C prototype generator
# To use this file, do
#
#	wmake /u /f watcom.mak

PROGRAM = cproto
DEFINES =
INCLUDES = -I.. -I..\porting

LEX = lex
YACC = yacc
CC = wcl386
CFLAGS = $(DEFINES) $(INCLUDES)
LIBS =

.c: ..

.c.obj: .AUTODEPEND
	$(CC) $(CFLAGS) /c $<

O = .obj
OBJECTS = \
	cproto$(O) \
	getopt$(O) \
	lintlibs$(O) \
	semantic$(O) \
	strkey$(O) \
	symbol$(O) \
	y_tab$(O)

all: $(PROGRAM).exe

$(PROGRAM).exe: $(OBJECTS)
	$(CC) $(CFLAGS) /fe=$@ $(OBJECTS) $(LIBS)

getopt$(O): ..\porting\getopt.c
	$(CC) $(CFLAGS) /c ..\porting\getopt.c

cproto.man: cproto.1
	cawf -man $*.1 | bsfilt - >$*.man

clean:
	-del $(PROGRAM).exe
	-del *$(O)
	-del *.err
	-del *.log
	-del *.out
	-del case*.c
