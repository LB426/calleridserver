cls
set DIR=RESULT\

echo %DIR%%RES%

if not exist "%DIR%" mkdir "%DIR%"
if exist "%DIR%\callsrvwnd.exe" del "%DIR%\callsrvwnd.exe"
if exist "%DIR%\callsrvwnd.obj" del "%DIR%\callsrvwnd.obj"
if exist "%DIR%\GENERIC.res" del "%DIR%\GENERIC.res"

cl -c -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl -nologo -D_X86_=1 -DWIN32 -D_WIN32 -D_WINNT -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0500 -DWINVER=0x0500 -D_MT -D_DLL -MD -Od /WX /Fo"%DIR%callsrvwnd.obj" callsrvwnd.c
Rc /r -DWIN32 -D_WIN32 -DWINVER=0x0500 -DDEBUG -D_DEBUG  /fo "%DIR%\GENERIC.res" GENERIC.rc
link /INCREMENTAL:NO /NOLOGO -subsystem:console,5.0 "%DIR%callsrvwnd.obj" "%DIR%\GENERIC.res" "callserver.lib" "serialfinder.lib" "client.lib" -out:%DIR%callsrvwnd.exe kernel32.lib ws2_32.lib mswsock.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib shell32.lib
