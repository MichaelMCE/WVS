@echo off

call setpath.bat

%DEVCRM% -f netclient.o netclient.dll libnetclient.a
SET IPPROTO=0
SET PLAYER=WINAMP

rem -e __DllMainCRTStartup@12

echo compiling....
%DEVCGCC% -I..\include\ -O3 -D_WIN32_WINNT=0x0500 -DBUILDDLL=1 -DIPPROTOCOL=%IPPROTO% -DBUILD_%PLAYER%=1 -march=i686 -mtune=generic -funroll-loops -c client.c -o netclient.o -I%DEVCI% -DBUILDING_DLL=1 %DEVOP%

echo linking...
%DEVCDLLW% --output-def netclient.def --implib libnetclient.a netclient.o -L%DEVCL% -lws2_32 --no-export-all-symbols --add-stdcall-alias -o netclient.dll -s

lib /def:netclient.def

rem echo cleaning....
del netclient.o
