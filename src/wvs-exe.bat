@echo off


del wvs.exe
call setpath.bat
@echo on

echo compiling....

rem -msse -mfpmath=sse 
%DEVCGCC% -Iinclude\ -c wa.c -o wvs.o -m32 -Werror -Wall -DIPPROTOCOL=0 -D_WIN32_WINNT=0x0501 -D_WIN32_IE=0x501 -D__WIN32__=1 -std=gnu99 -O3 -mmmx -march=i686 -mtune=generic -fwhole-program -ffast-math -fgnu89-inline -funroll-loops -finline-functions -fomit-frame-pointer -pipe -s
windres -i wvs.rc -o wvs.res.o --target=pe-i386
%DEVCGCC% wvs.o wvs.res.o -o wvs.exe -m32 -mwindows -lm lib\libmylcd.a lib\liblglcd.a -liphlpapi -lwininet -lwsock32 -lwinmm -lpsapi -lws2_32 -lhid -lsetupapi lib/libhook.a -s

strip wvs.exe

del wvs.o
del wvs.res.o 
