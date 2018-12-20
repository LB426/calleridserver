SET MINGW=E:\TDM-GCC-32-5.1.0\bin
SET BOOST_INC=E:\boost_1_61_0_TDM-GCC-32-5_1_0\include
SET BOOST_LIB=E:\boost_1_61_0_TDM-GCC-32-5_1_0\lib
SET PATH=%BOOST_LIB%;%MINGW%;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;

if exist serialmodem.o del serialmodem.o
if exist main.o del main.o
if exist main.exe del main.exe

g++ -c -Wall -D _WIN32_WINNT=0x0501 -I%BOOST_INC% -o serialmodem.o serialmodem.cpp
g++ -c -D _WIN32_WINNT=0x0501 -I%BOOST_INC% -o main.o main.cpp

g++ -o main.exe main.o serialmodem.o -L%BOOST_LIB% -lboost_system-mgw51-mt-1_61 -lboost_thread-mgw51-mt-1_61 -lboost_regex-mgw51-mt-1_61.dll -lboost_locale-mgw51-mt-1_61.dll -lws2_32 -lwsock32
