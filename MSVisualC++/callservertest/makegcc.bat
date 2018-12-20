CLS
SET INCLUDE=C:\mingw\include
SET LIB=C:\mingw\lib
SET DIR=RESULT\
ECHO %DIR%%RES%
if NOT EXIST "%DIR%" MKDIR "%DIR%"

gcc-2 -Wall -ansi -pedantic -o %DIR%test.o -c test.c
gcc-2 -s -o %DIR%test %DIR%test.o -lwsock32 -lKernel32
