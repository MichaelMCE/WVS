@echo off

cd ..
call setpath.bat
cd xmplay


%DEVCRM% -f xmplay.o xmplay.dll libxmplay.a
SET IPPROTO=0
SET PLAYER=XMPLAY

rem -e __DllMainCRTStartup@12

echo compiling....
%DEVCGCC% -O3 -D_WIN32_WINNT=0x0500 -DIPPROTOCOL=%IPPROTO% -DBUILD_%PLAYER%=1 -march=i686 -mtune=generic -funroll-loops -c xmplay.c -o xmplay.o -I%DEVCI% %DEVOP%

echo linking...
%DEVCDLLW% --output-def xmplay.def --implib libxmplay.a xmplay.o -L%DEVCL% -lwinmm -lws2_32 --no-export-all-symbols --add-stdcall-alias -o xmp-wvs.dll -s

rem echo cleaning....
del xmplay.o

copy "xmp-wvs.dll" "h:/Program Files/xmplay/xmp-wvs.dll" /y

