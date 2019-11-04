@echo off


REM this file is called from withn 'BUILD-Plugins.bat'

call setpath.bat

%DEVCRM% -f gen_wamp.o gen_wamp.dll

echo compiling....
%DEVCGCC% -I..\include\ -O2 -ffast-math -fgnu89-inline -fomit-frame-pointer -DIPPROTOCOL=%IPPROTO% -DBUILD_%PLAYER%=1 -DCOBJMACROS -march=i686 -mtune=generic -finline-functions -funroll-loops -c gen_wamp/gen_wamp.c -o gen_wamp.o -I%DEVCI% -DBUILDING_DLL=1 %DEVOP%


rem -DIPPROTOCOL=%IPPROTO% -DBUILD_%PLAYER%=1 

echo linking...
%DEVCDLLW% --output-def libwamp.def --implib libgen_wamp.a gen_wamp.o -lwinmm -lwininet -lm -lole32 -luuid -loleaut32 -lwsock32 -lws2_32 -L%DEVCL% --no-export-all-symbols --add-stdcall-alias -o gen_wamp.dll -s

rem echo cleaning....
del gen_wamp.o
