# $Id: descrip.mms,v 4.1 1998/01/20 00:59:43 cthuang Exp $
# VMS make-file for CPROTO test directory
#

####### (Development) ##########################################################

####### (Command-line Options) #################################################

####### (Standard Lists) #######################################################

REF_FILES=\
	case1.ref	case2.ref	case3.ref	case4.ref\
	case5.ref	case6.ref	case7.ref	case8.ref\
	case9.ref	case10.ref	case11.ref	case12.ref\
	case13.ref	case14.ref	case15.ref\
	case17.ref	case18.ref	case19.ref	case20.ref\
	case21.ref

COM_FILES=\
	case1.dcl	case2.dcl	case3.dcl	case4.dcl\
	case5.dcl	case6.dcl	case7.dcl	case8.dcl\
	case9.dcl	case10.dcl	case11.dcl	case12.dcl\
	case13.dcl	case14.dcl	case15.dcl\
	case17.dcl	case18.dcl	case19.dcl	case20.dcl\
	case21.dcl

TESTDATA=\
	syntax.c\
	$(COM_FILES)\
	$(REF_FILES)

SCRIPTS	=\
	run_tests.sh	run_tests.com

SOURCES	= makefile descrip.mms $(SCRIPTS) $(TESTDATA)

ALL	= $(SCRIPTS)

####### (Standard Productions) #################################################
all :	$(ALL) [-.bin]$(THIS).exe [-.bin]$(THIS)1.exe
	@ write sys$output "Type ""mms run_tests""

clean :
	@- write sys$output "** made $@"
clobber :	clean
	@- write sys$output "** made $@"
destroy :
	@- remove -vf *.*;*
run_tests :	$(SCRIPTS) $(TESTDATA)
	@run_tests

####### (Details of Productions) ###############################################
