cls
set DIR=RESULT\

echo %DIR%

if not exist "%DIR%" mkdir "%DIR%"

cl -c -DCRTAPI1=_stdcall -DCRTAPI2=_stdcall -nologo -D_X86_=1  -DWIN32 -D_WIN32 -W3 -D_WINNT -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0500 -DWINVER=0x0500  -D_MT -D_DLL -MD -Od /W3 /Fo"%DIR%client.obj" client.c
lib -machine:i386 -def:client.def "%DIR%client.obj" -out:"%DIR%client.lib"
link /INCREMENTAL:NO /NOLOGO -entry:_DllMainCRTStartup@12 -dll -base:0x1C000000 -out:"%DIR%client.dll" "%DIR%client.exp" "%DIR%client.obj" kernel32.lib ws2_32.lib mswsock.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib
