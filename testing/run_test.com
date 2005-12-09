$! $Id: run_test.com,v 4.3 1998/01/08 00:03:35 cthuang Exp $
$! Perform all regression-tests for CPROTO on VAX/VMS
$
$	verify	= F$VERIFY(0)
$	path	:= 'F$ENVIRONMENT("DEFAULT")
$	path	:= "''F$EXTRACT(0,F$LENGTH(path)-1,path)'.-]"
$	CPROTO	:== "$''path'cproto.exe"
$	CPROTO1	:== "$''path'cproto1.exe"
$
$	caselist=""
$	open/read input_file run_test.txt
$ read_loop:
$	read/end=read_exit input_file sometext
$	if f$locate("vms",sometext) .ne. f$length(sometext)
$	then
$		sometext = f$edit(sometext,"collapse,lowercase")
$		sometext = f$extract(0, f$locate("=", sometext), sometext)
$		caselist = "''caselist'/''sometext'"
$	endif
$	goto read_loop
$ read_exit:
$	close input_file
$	caselist = "''caselist'/"
$
$
$	if "''p1'" .eqs. ""
$	then
$		cases = "*"
$	else
$		cases = "''p1'"
$	endif
$
$	old_name = ""
$ test_loop:
$	ref_name = F$SEARCH("''cases'.ref;",1)
$	if ref_name .eqs. ""
$	then
$		verify = F$VERIFY(verify)
$		exit
$	endif
$	if "''old_name'" .eqs. "''ref_name'" then exit
$	old_name = ref_name
$
$	com_name = F$EXTRACT(0, F$LOCATE(".REF;", ref_name), ref_name)
$	casename = F$PARSE(ref_name,,, "NAME", "SYNTAX_ONLY")
$	casename = F$EDIT(casename,"LOWERCASE")
$	case_num = F$EXTRACT(4, F$LENGTH(casename), casename)
$
$	write sys$output "** ",ref_name
$	if f$locate("/''casename'/", caselist) .eq. f$length(caselist)
$	then
$		write sys$output "...skipped"
$		goto test_loop
$	endif
$
$	if F$SEARCH("''com_name'.dcl;", 2) .eqs. ""
$	then
$		write sys$output "?? no script for ", ref_name
$	else
$		if F$SEARCH("''com_name'.out") .nes. "" then delete 'com_name.out;*
$		if F$SEARCH("''com_name'.err") .nes. "" then delete 'com_name.err;*
$		if F$SEARCH("''casename'.c") .nes. "" then delete 'casename.c;*
$		copy syntax.c  'casename.c;
$		set noon
$		define/user_mode sys$error 'casename.err
$		@'com_name.dcl
$		set on
$		if F$SEARCH("''com_name'.out") .eqs. ""
$		then
$			write sys$output "?? no output from test"
$		else
$			if f$search("''casename'.c;-1") .nes. ""
$			then
$				diff/out='com_name 'casename.c
$				if $severity .ne. 1
$				then
$					open/append output_file 'com_name.out
$					write output_file "... edited ''casename'.c ..."
$					close output_file
$					append 'casename.c 'com_name.out
$				endif
$			endif
$			diff/out='com_name 'com_name.out 'ref_name
$			if $severity .ne. 1
$			then
$				write sys$output "?? test failed"
$				type 'com_name.dif
$			else
$				delete 'com_name.dif;*
$				if F$SEARCH("''com_name'.err") .nes. "" then delete 'com_name.err;*
$			endif
$			delete 'casename.c;*, 'com_name.out;*
$		endif
$	endif
$	goto test_loop
