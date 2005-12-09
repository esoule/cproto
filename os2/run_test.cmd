/* REXX script to run cproto tests. */
/* $Id: run_test.cmd,v 4.2 1998/01/22 19:49:38 cthuang Exp $ */
"@echo off"

testDir = "..\testing"
casesFile = testDir||"\run_test.txt"
sourceFile = testDir||"\syntax.c"
logFile = "run_test.out"

echo "Test log" ">" logFile

linein(casesFile,1,0);
do while lines(casesFile) > 0
    parse value linein(casesFile) with caseID '=' platforms '=' options
    if left(caseID, 4) = "CASE" & pos("os2", platforms) > 0 then do
	say caseID

	baseName = "case"||substr(caseID, 6, 2)
        inputFile = baseName||".c"
	outputFile = baseName||".out"
	refFile = testDir||"\"||baseName||".ref"

	copy sourceFile inputFile ">nul"
	cproto options inputFile ">" outputFile
	echo n "|" comp outputFile refFile ">>" logFile
    end
end
