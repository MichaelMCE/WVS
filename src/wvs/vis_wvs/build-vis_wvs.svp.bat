@echo off

call setpath.bat

%DEVCRM% -f vis_wvs.o vis_wvs.svp

echo compiling....
%DEVCGCC% -O3 -DIPPROTOCOL=0 -DBUILD_AIMP=1 -march=i686 -mtune=generic -funroll-loops -c vis_wvs.c -o vis_wvs.o -I%DEVCI% -DBUILDING_DLL=1 %DEVOP%

echo linking...
%DEVCDLLW% --output-def libvis_wvs.def --implib libvis_wvs.a vis_wvs.o -lwininet -lm -lwsock32 -L%DEVCL% --no-export-all-symbols --add-stdcall-alias -o vis_wvs.svp -s

rem echo cleaning....
del vis_wvs.o