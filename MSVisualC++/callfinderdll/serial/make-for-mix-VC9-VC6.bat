cls
set PATH=%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem;C:\MSVS6\Common\MSDev98\Bin;C:\MSVS6\VC98\Bin
set INCLUDE=C:\Program Files\Microsoft SDKs\Windows\v6.1\Include;C:\MSVS6\VC98\Include;C:\Program Files\Microsoft Visual Studio 9.0\VC\include
set LIB=C:\Program Files\Microsoft SDKs\Windows\v6.1\Lib;C:\MSVS6\VC98\Lib
set DIR=RESULT\

echo %DIR%

if not exist "%DIR%" mkdir "%DIR%"

cl -c -DCRTAPI1=_stdcall -DCRTAPI2=_stdcall -nologo -D_X86_=1 -DWIN32 -D_WIN32 -D_WINNT -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0500 -DWINVER=0x0500 -D_MT -D_DLL /MD -Od /Fo"%DIR%main.obj" main.c
lib -machine:i386 -def:main.def "%DIR%main.obj" -out:"%DIR%serialfinder.lib"
link /INCREMENTAL:NO /NOLOGO -entry:_DllMainCRTStartup@12 -dll -base:0x1C000000 -out:"%DIR%serialfinder.dll" "%DIR%serialfinder.exp" "%DIR%main.obj" kernel32.lib ws2_32.lib mswsock.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib
