@echo off

call setpath.bat

%DEVCRM% -f plugintemplate.o plugintemplate.dll libplugintemplate.a

rem -e __DllMainCRTStartup@12

echo compiling....
%DEVCGCC% -O3 -march=i686 -mtune=generic -funroll-loops -c plugintemplate.c -o plugintemplate.o -I%DEVCI% -DBUILDING_DLL=1 %DEVOP%

echo linking...
%DEVCDLLW% --implib libplugintemplate.a plugintemplate.o -lmylcd -L%DEVCL% --no-export-all-symbols --add-stdcall-alias -o plugintemplate.dll -s

rem echo cleaning....
del plugintemplate.o
del libplugintemplate.a
