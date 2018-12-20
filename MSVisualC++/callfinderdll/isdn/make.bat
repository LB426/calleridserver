cls
set DIR=RESULT\

echo %DIR%

if not exist "%DIR%" mkdir "%DIR%"

cl -c -nologo -W3 -MD -Og -G6 -TP /Fo"%DIR%capifunc.obj" CapiFunc.cpp
cl -c -DCRTAPI1=_stdcall -DCRTAPI2=_stdcall -nologo -D_X86_=1  -DWIN32 -D_WIN32 -W3 -D_WINNT -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0500 -DWINVER=0x0500  -D_MT -D_DLL -MD -Od /W3 /Fo"%DIR%isdnfinder.obj" isdnfinder.cpp
lib -machine:i386 -def:isdnfinder.def "%DIR%isdnfinder.obj" -out:"%DIR%isdnfinder.lib"
link /INCREMENTAL:NO /NOLOGO -entry:_DllMainCRTStartup@12 -dll -base:0x1C000000 -out:"%DIR%isdnfinder.dll" "%DIR%isdnfinder.exp" "%DIR%isdnfinder.obj" "%DIR%capifunc.obj" kernel32.lib ws2_32.lib mswsock.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib
