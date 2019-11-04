@echo off


del wvs.exe
rem call setpath.bat

echo compiling....
 
rem -msse -mfpmath=sse -fgnu89-inline -mwindows -fwhole-program -pipe -mwindows -D__NOHOOK__=1 

%DEVCGCC% -Iinclude\ -g -c wa.c -o wvs.o -Werror -fgnu89-inline -Werror -Wall -DIPPROTOCOL=0 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x500 -D__WIN32__=1 -D__DEBUG__=1 -march=i686 -mtune=generic -fwhole-program
windres -i wvs.rc -o wvs.res.o
%DEVCGCC% wvs.o wvs.res.o -o wvs.exe -lm lib\libmylcd.a -liphlpapi -lwininet -lwsock32 -lwinmm -lpsapi -lws2_32 -lhid -lsetupapi lib/libhook.a

rem lib\liblglcd.a 

del wvs.o
del wvs.res.o 
