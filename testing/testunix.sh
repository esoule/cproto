#!/bin/sh
# $Id: testunix.sh,v 4.1 1998/01/20 00:59:47 cthuang Exp $
#
# Test one or more given cases by number, creating the VMS test script
# as a side-effect.
#
CPROTO=../cproto
for i in $*
do
	echo '** Case '$i
	I="case$i"
	./make_dcl.sh $i

	rm -f $I.c $I.out $I.err $I.tmp
	cp syntax.c $I.c
	chmod +w $I.c

	. ./$I.dcl 2>$I.err

	if [ -f $I.ref ]
	then
		# When we select either the -a or -t options, CPROTO will edit
		# the input file.  It also writes to the standard output the
		# prototypes that it is extracting.  Since there's only one
		# reference for each test, I simply concatenate the edited file
		# after the test output, for use as a combined reference.
		if ( cmp -s $I.c syntax.c )
		then
			rm -f $I.c
		else
			echo '... edited '$I.c' ...' >>$I.out
			cat $I.c >>$I.out
			rm -f $I.c
		fi

		if [ -f $I.out ]
		then
			diff -b -c $I.ref $I.out |fgrep -v 'No diff' >$I.tmp
			if [ -s $I.tmp ]
			then
				cat $I.err
				cat $I.tmp
			else
				echo '... ok'
				rm -f $I.out $I.tmp $I.err
			fi
		else
			echo '? no output '$I
		fi
	else
		echo '...saving reference for '$i
		mv $I.out $I.ref
		rm -f $I.err
	fi
done
