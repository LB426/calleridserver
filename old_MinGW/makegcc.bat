CLS
SET INCLUDE=C:\mingw\include
SET LIB=C:\mingw\lib
SET DIR=RESULT\
ECHO %DIR%%RES%
if NOT EXIST "%DIR%" MKDIR "%DIR%"

gcc-2 -Wall -ansi -pedantic -o %DIR%test.o -c test.c
gcc-2 -s -o %DIR%test %DIR%test.o -lwsock32

gcc-2 -Wall -ansi -pedantic -o %DIR%heap.o -c heap.c
gcc-2 -Wall -o %DIR%callserver.o -c callserver.cpp
gcc-2 -Wall -o %DIR%main.o -c main.cpp
gcc-2 -s -o %DIR%callserver %DIR%callserver.o %DIR%heap.o %DIR%main.o -lwsock32
