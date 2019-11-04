@echo off

call setpath.bat

%DEVCRM% -f vis_wamp.o vis_wamp.dll libvis_wamp.a libwamp.def

rem -e __DllMainCRTStartup@12

echo compiling....
%DEVCGCC% -I..\include\ -O2 -msse -mfpmath=sse -ffast-math -fgnu89-inline -finline-functions -fomit-frame-pointer -DIPPROTOCOL=%IPPROTO% -DBUILD_%PLAYER%=1 -march=i686 -mtune=generic -funroll-loops -c vis_wamp\vis_wamp.c -o vis_wamp.o -I%DEVCI% -DBUILDING_DLL=1 %DEVOP%


rem -DIPPROTOCOL=%IPPROTO% -DBUILD_%PLAYER%=1 

echo linking...
%DEVCDLLW% --output-def libwamp.def --implib libvis_wamp.a vis_wamp.o -lwininet -lm -lwsock32 -L%DEVCL% --no-export-all-symbols --add-stdcall-alias -o vis_wamp.dll -s

rem echo cleaning....
del vis_wamp.o
