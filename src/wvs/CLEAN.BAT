@echo off


del *.bak
del *.o
del *.s
del *.def
rem del *.a
del *.layout
rem del mylcd.dll
del libmylcd.a
del libvis_wvs.a
rem del libmylcd.a
del libvis_wamp.a
del vis_wamp.dll
del gen_wamp.dll
del libgen_wamp.a
del *.bmp
del *.tga
del libvis.a
del *.pgm
del *.png
del *.svp
del *.raw
rem del libhook*.*
del hook.dll
del plugintemplate.dll
del libplugintemplate.a
del *.exe
del netclient.dll
del *.exp
del netclient.lib
del *.backup

del CopyToWinampPlugins\gen_wamp.dll
del CopyToWinampPlugins\vis_wamp.dll
rem del CopyToWinampPlugins\TCP_CopyToWinampPlugins\gen_wamp.dll
rem del CopyToWinampPlugins\TCP_CopyToWinampPlugins\vis_wamp.dll
rem del CopyToWinampPlugins\TCP_CopyToWinampPlugins\tcp_readme.txt

del CopyToMediaMonkeyPlugins\gen_wamp.dll
del CopyToMediaMonkeyPlugins\vis_wamp.dll

del CopyToAimpPlugins\gen_wamp.dll
del CopyToAimpPlugins\vis_wvs.svp

del CopyToXMPlay\xmp-wvs.dll
del CopyToXMPlay\vis_wvs.svp
del CopyToXMPlay\xmp-wvsclient.dll

rem rmdir CopyToWinampPlugins\TCP_CopyToWinampPlugins
rmdir CopyToWinampPlugins
rmdir CopyToMediaMonkeyPlugins
rmdir CopyToAimpPlugins
rmdir CopyToXMPlay





