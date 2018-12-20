cls
set DIR=RESULT\

echo %DIR%%RES%

if not exist "%DIR%" mkdir "%DIR%"

cl -c -DCRTAPI1=_stdcall -DCRTAPI2=_stdcall -nologo -D_X86_=1  -DWIN32 -D_WIN32 -W3 -D_WINNT -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0500 -DWINVER=0x0500  -D_MT -MD -Od /WX /Fo"%DIR%runcallserver.obj" runcallserver.c
link /INCREMENTAL:NO /NOLOGO -subsystem:console,5.0 -out:"%DIR%runcallserver.exe" "%DIR%runcallserver.obj" "%DIR%callserver.lib" kernel32.lib ws2_32.lib mswsock.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib
