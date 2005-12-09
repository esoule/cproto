#!/bin/sh
# $Id: run_test.sh,v 4.3 1998/01/08 00:03:37 cthuang Exp $
#
# This test-script assumes that we can reproduce (except for minor whitespace)
# the reference files listed in run_test.txt
#
echo '** '`date`
for i in `grep 'CASE.*unix' run_test.txt | sed -e 's/^[^0-9]*//'  -e 's/[ 	].*//'`
do
	./testunix.sh $i
done
