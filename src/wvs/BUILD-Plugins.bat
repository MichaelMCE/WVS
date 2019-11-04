@echo off

rem IPPROTOCOL: 0=udp, 1=tcp


@echo **** Winamp ****
SET IPPROTO=0
SET PLAYER=WINAMP
call vis_wamp.dll.bat
call gen_wamp.dll.bat
mkdir CopyToWinampPlugins
copy gen_wamp.dll CopyToWinampPlugins\gen_wamp.dll
copy vis_wamp.dll CopyToWinampPlugins\vis_wamp.dll



@echo **** MediaMonkey ****
SET IPPROTO=0
SET PLAYER=MEDIAMONKEY
call vis_wamp.dll.bat
call gen_wamp.dll.bat
mkdir CopyToMediaMonkeyPlugins
copy gen_wamp.dll CopyToMediaMonkeyPlugins\gen_wamp.dll
copy vis_wamp.dll CopyToMediaMonkeyPlugins\vis_wamp.dll



@echo **** Aimp2 ****
SET IPPROTO=0
SET PLAYER=SVP
call vis_wvs.svp.bat
call gen_wamp.dll.bat
mkdir CopyToAimpPlugins
copy gen_wamp.dll CopyToAimpPlugins\gen_wamp.dll
copy vis_wvs.svp CopyToAimpPlugins\vis_wvs.svp



@echo **** XMPlay****
SET IPPROTO=0
SET PLAYER=XMPLAY
cd xmplay
call build.bat
call BUILD-xmp-wvsclient.bat
cd ..
mkdir CopyToXMPlay
copy xmplay\xmp-wvsclient.dll CopyToXMPlay\xmp-wvsclient.dll
copy xmplay\xmp-wvs.dll CopyToXMPlay\xmp-wvs.dll
copy vis_wvs.svp CopyToXMPlay\vis_wvs.svp






