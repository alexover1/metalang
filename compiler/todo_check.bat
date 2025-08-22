@echo off

set Wildcard=*.h *.cpp *.inl *.c

findstr -s -n -i -l "TODO" %Wildcard%
