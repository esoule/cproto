@echo off
rem $Id: test_dos.bat,v 4.1 1998/01/08 00:19:27 cthuang Exp $
erase run_test.out
for %%n in ( case*.bat ) do call %%n
