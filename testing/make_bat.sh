#!/bin/sh
# $Id: make_bat.sh,v 4.3 1998/01/24 01:42:13 cthuang Exp $
#
# This makes a special ".bat" file for testing CPROTO on MSDOS.
# It won't work properly if /bin/echo tries to expand the backslash sequences.
# (This is the case on SunOS 4.1.x).
#
# The MSDOS 'fc' program doesn't return an exit code for differing files.
# We have to examine the output log...
TMP=make$$
rm -f $TMP
trap "rm -f $TMP" 0 1 2 5 15
if [ -f /bin/echo ]
then
	ECHO=/bin/echo
else
	ECHO=echo
fi
for i in $*
do
	i="`echo $i | sed -e 's/[A-Za-z_.]//g'`"
	I="case$i"
	if [ -n "$i" -a ! -f $I.bat ]
	then
		echo '** making '$I.bat
		cat >$I.bat <<EOF
@echo off
echo Testing case $i
echo Testing case $i >>run_test.out
if exist $I.out erase $I.out
if exist $I.dif erase $I.dif
copy syntax.c $I.c
EOF
		grep 'CASE[ 	]'$i run_test.txt >$TMP
		OPTS=`sed -e 's/^.*=/CPROTO/' $TMP`
		TEST=`sed -e 's/^.*=/CPROTO/' -e 's/-[at]//' $TMP`
		sed	-e 's/^.*=/CPROTO/' $TMP | \
		sed	-e 's/^/..\\/' \
			-e s/\$/\ -o$I.out\ -O$I.err\ $I.c/ \
			-e s/\"//g >>$I.bat
		if [ ".$OPTS" !=  ".$TEST" ]
		then
		cat >>$I.bat <<EOF
echo ... edited $I.c ... >>$I.out
type $I.c >>$I.out
EOF
		fi
		cat >>$I.bat <<EOF
if not exist $I.ref goto saveit
    fc/L/n/w $I.out $I.ref >>run_test.out
    if exist $I.out erase $I.out
    if exist $I.err erase $I.err
    goto end
:saveit
    ren $I.out $I.ref
:end
    erase $I.c
EOF
	fi
done
