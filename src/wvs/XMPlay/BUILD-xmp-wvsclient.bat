@echo off


del xmp-wvsclient.dll

cd ..
call setpath.bat
cd xmplay


echo compiling....

SET IPPROTO=0
SET PLAYER=XMPLAY

rem -msse -mfpmath=sse --implib libxmplay.a  -Werror -Wall 
%DEVCGCC% -I..\..\include\ -DIPPROTOCOL=0 -DBUILD_XMPLAY=1 -DCOBJMACROS -D_WIN32_WINNT=0x0500 -D__WIN32__=1 -D_WIN32_IE=0x500 -std=gnu99 -O1 -march=i586 -mtune=generic -ffast-math -fgnu89-inline -funroll-loops -finline-functions -fomit-frame-pointer -c xmpclient.c -o xmpclient.o -pipe -I%DEVCI% -DBUILDING_DLL=1 %DEVOP%
windres -i "..\..\wvs.rc" -o wvs.res.o

@echo linking...
%DEVCDLLW% --output-def libxmplay.def wvs.res.o xmpclient.o -lwinmm -lwininet -lm ..\..\lib\libmylcdstatic.a ..\..\lib\liblglcd.a -lpsapi -liphlpapi -lhid -lole32 -luuid -loleaut32 -lwsock32 -lws2_32 -lsetupapi ..\..\lib\libhook.a -L%DEVCL% --no-export-all-symbols --add-stdcall-alias -o xmp-wvsclient.dll -s

del xmpclient.o
del wvs.res.o

copy "xmp-wvsclient.dll" "h:/Program Files/xmplay/xmp-wvsclient.dll" /y
