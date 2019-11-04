@echo off


del gen_wvs.dll
call setpath.bat

echo compiling....

rem -msse -mfpmath=sse --implib libgen_wvs.a  -fomit-frame-pointer
%DEVCGCC% -Iinclude\ -g -DIPPROTOCOL=0 -DBUILD_MEDIAMONKEY=1 -DCOBJMACROS -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x500 -D__WIN32__=1 -std=gnu99 -fgnu89-inline -funroll-loops -finline-functions -c gen_wvs.c -o gen_wvs.o -pipe -I%DEVCI% -DBUILDING_DLL=1
windres -i wvs.rc -o wvs.res.o

@echo linking...
%DEVCDLLW% --output-def libgen_wvs.def gen_wvs.o wvs.res.o -lwinmm -lwininet -lm lib\libmylcdstatic.a lib\liblglcd.a -lpsapi -liphlpapi -lhid -lole32 -luuid -loleaut32 -lwsock32 -lws2_32 -lsetupapi lib/libhook.a -L%DEVCL% --no-export-all-symbols --add-stdcall-alias -o gen_wvs.dll

del gen_wvs.o
del wvs.res.o 
