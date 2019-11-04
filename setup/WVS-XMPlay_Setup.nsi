
; NSIS installer created by Alex / Pytte
; Maintained by Michael McElligott
; http://mylcd.sourceforge.net

!define PRODUCT_NAME "WVS"
!define PRODUCT_VERSION "0.30.14 for XMPlay"
!define PRODUCT_PUBLISHER "Michael McElligott"
!define PRODUCT_WEB_SITE "http://mylcd.sourceforge.net"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\wvs.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON   "..\src\Icons\WVS_XP_Rounded_Square.ico"
!define MUI_UNICON "..\src\Icons\WVS_XP_Rounded_Square.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "t:\wvssetup\wvs\license.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "WVS"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\XMPlay.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\wvs\readme.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------


Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "wvs_03014-xmplay-setup.exe"
InstallDir "$PROGRAMFILES\XMPlay"
;InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
InstallDirRegKey HKLM "Software\WVS" "Install_Dir"
ShowInstDetails show
ShowUnInstDetails show

Function ClosePlayer
Push $5

loop:
  push "xmplay.exe"
  processwork::existsprocess
  pop $5
  IntCmp $5 0 done

  MessageBox MB_RETRYCANCEL|MB_ICONSTOP 'XMPlay must be closed during this installation.$\r$\n Close XMPlay now, or press "Retry" to automatically close XMPlay and continue or press "Cancel" to cancel the installation entirely.' IDCANCEL BailOut
  push "xmplay.exe"
  processwork::KillProcess
  Sleep 1000
Goto loop

BailOut:
  Abort

done:
Pop $5
FunctionEnd

Section "MainSection" SEC01


; The stuff to install
  ; attempt to close Player if it's running
  Call ClosePlayer
  ; add a small delay to allow any file operations to happen once Winamp
  ; is closed

  SetOverwrite try
  SetOutPath "$INSTDIR"
;  File "t:\wvssetup\wvs\CopyToWinampPlugins\gen_wamp.dll"
;  File "t:\wvssetup\wvs\CopyToWinampPlugins\vis_wamp.dll"
;  File "t:\wvssetup\wvs\CopyToWinampPlugins\gen_wvs.dll"
    
;  SetOutPath "$INSTDIR\wvs\CopyToMediaMonkeyPlugins"
;  File "t:\wvssetup\wvs\CopyToMediaMonkeyPlugins\gen_wamp.dll"
;  File "t:\wvssetup\wvs\CopyToMediaMonkeyPlugins\vis_wamp.dll"
  
;  SetOutPath "$INSTDIR\wvs\CopyToWinampPlugins"
;  File "t:\wvssetup\wvs\CopyToWinampPlugins\gen_wamp.dll"
;  File "t:\wvssetup\wvs\CopyToWinampPlugins\vis_wamp.dll"
  
;  SetOutPath "$INSTDIR\wvs\CopyToAimpPlugins"
;  File "t:\wvssetup\wvs\CopyToAimpPlugins\gen_wamp.dll"
;  File "t:\wvssetup\wvs\CopyToAimpPlugins\vis_wvs.svp"

;  SetOutPath "$INSTDIR\wvs\CopyToXMPlay"
  File "t:\wvssetup\wvs\CopyToXMPlay\xmp-wvs.dll"
  File "t:\wvssetup\wvs\CopyToXMPlay\vis_wvs.svp"
  File "t:\wvssetup\wvs\CopyToXMPlay\xmp-wvsclient.dll"
    
  SetOutPath "$INSTDIR\wvs\CopyToAmsnScripts_0.97rc1"
  File "t:\wvssetup\wvs\CopyToAmsnScripts_0.97rc1\remote.tcl"
      
  SetOutPath "$INSTDIR\wvs\fonts\bdf"
  File "t:\wvssetup\wvs\fonts\bdf\5x7.bdf"
  File "t:\wvssetup\wvs\fonts\bdf\b16_b.bdf"
  File "t:\wvssetup\wvs\fonts\bdf\rought18.bdf"
  File "t:\wvssetup\wvs\fonts\bdf\snap.bdf"
  File "t:\wvssetup\wvs\fonts\bdf\wenquanyi_9pt.bdf"
  File "t:\wvssetup\wvs\fonts\bdf\helvr12.bdf"
  
  SetOutPath "$INSTDIR\wvs\fonts\tga"
  File "t:\wvssetup\wvs\fonts\tga\arial.tga"
  File "t:\wvssetup\wvs\fonts\tga\comicsansms7x8.tga"
  File "t:\wvssetup\wvs\fonts\tga\couriernewce8.tga"
  File "t:\wvssetup\wvs\fonts\tga\dotumche24x24.tga"
  File "t:\wvssetup\wvs\fonts\tga\interdimensional16.tga"
  File "t:\wvssetup\wvs\fonts\tga\smallfonts7x7.tga"
  
  SetOutPath "$INSTDIR\wvs\mappings"
  File "t:\wvssetup\wvs\mappings\8859-1"
  File "t:\wvssetup\wvs\mappings\8859-10"
  File "t:\wvssetup\wvs\mappings\8859-11"
  File "t:\wvssetup\wvs\mappings\8859-13"
  File "t:\wvssetup\wvs\mappings\8859-14"
  File "t:\wvssetup\wvs\mappings\8859-15"
  File "t:\wvssetup\wvs\mappings\8859-16"
  File "t:\wvssetup\wvs\mappings\8859-2"
  File "t:\wvssetup\wvs\mappings\8859-3"
  File "t:\wvssetup\wvs\mappings\8859-4"
  File "t:\wvssetup\wvs\mappings\8859-5"
  File "t:\wvssetup\wvs\mappings\8859-6"
  File "t:\wvssetup\wvs\mappings\8859-7"
  File "t:\wvssetup\wvs\mappings\8859-8"
  File "t:\wvssetup\wvs\mappings\8859-9"
  File "t:\wvssetup\wvs\mappings\cp1250"
  File "t:\wvssetup\wvs\mappings\cp1251"
  File "t:\wvssetup\wvs\mappings\cp936"
  File "t:\wvssetup\wvs\mappings\cp949"
  File "t:\wvssetup\wvs\mappings\cp950"
  File "t:\wvssetup\wvs\mappings\cp850"
  
  SetOutPath "$INSTDIR\wvs\images"
  File "t:\wvssetup\wvs\images\msn1.tga"
  File "t:\wvssetup\wvs\images\mail.tga"
  
;  SetOutPath "$INSTDIR\wvs\libmylcd"
;  File "t:\wvssetup\wvs\libmylcd\COPYING.LIB"
;  File "t:\wvssetup\wvs\libmylcd\mylcd.h"
;  File "t:\wvssetup\wvs\libmylcd\mylcdtypes.h"
;  File "t:\wvssetup\wvs\libmylcd\mylcdconfig.h"
;  File "t:\wvssetup\wvs\libmylcd\libmylcd.a"
;  File "t:\wvssetup\wvs\libmylcd\libmylcdstatic.a"
;  File "t:\wvssetup\wvs\libmylcd\mylcd.lib"
  
  SetOutPath "$INSTDIR\"
  File "t:\wvssetup\wvs\mylcd.dll"
  File "t:\wvssetup\wvs\hook.dll"
  
  SetOutPath "$INSTDIR\wvs"
;  File "t:\wvssetup\wvs\mylcd.dll"
;  File "t:\wvssetup\wvs\hook.dll"
;  File "t:\wvssetup\wvs\liesmich.txt"
;  File "t:\wvssetup\wvs\nonAutoStart.vbs"
;  File "t:\wvssetup\wvs\MM_Auto.vbs"
;  File "t:\wvssetup\wvs\wvs.exe"

  File "t:\wvssetup\wvs\license.txt"
  File "t:\wvssetup\wvs\mylcd.cfg"
  File "t:\wvssetup\wvs\wvs.cfg"
  File "t:\wvssetup\wvs\profile.cfg"
  File "t:\wvssetup\wvs\manual.pdf"
  File "t:\wvssetup\wvs\readme.txt"
  File "t:\wvssetup\wvs\changelog.txt"
  File "t:\wvssetup\wvs\COPYING.LIB"
  File "t:\wvssetup\wvs\netClient.zip"


; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
;  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\WVS.lnk" "$INSTDIR\wvs\wvs.exe"
;  CreateShortCut "$DESKTOP\WVS.lnk" "$INSTDIR\wvs\wvs.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -AdditionalIcons
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteIniStr "$INSTDIR\wvs\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Readme.lnk" "$INSTDIR\wvs\readme.txt"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Changelog.lnk" "$INSTDIR\wvs\changelog.txt"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Client example (SDK).lnk" "$INSTDIR\wvs\netClient.zip"
  
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Website.lnk" "$INSTDIR\wvs\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk" "$INSTDIR\wvs_uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -Post
  WriteRegStr HKLM SOFTWARE\WVS "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\wvs\wvs.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\wvs\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\wvs\wvs.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteUninstaller "$INSTDIR\wvs_uninstall.exe"
SectionEnd

Section "Uninstall"
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  Delete "$INSTDIR\wvs\${PRODUCT_NAME}.url"
  
  Delete "$INSTDIR\wvs\hook.dll"
  Delete "$INSTDIR\wvs\license.txt"
  Delete "$INSTDIR\wvs\mylcd.cfg"
  Delete "$INSTDIR\wvs\wvs.cfg"
  Delete "$INSTDIR\wvs\profile.cfg"
  Delete "$INSTDIR\wvs\manual.pdf"
  Delete "$INSTDIR\wvs\mylcd.dll"
  Delete "$INSTDIR\wvs\readme.txt"
  Delete "$INSTDIR\wvs\changelog.txt"
  Delete "$INSTDIR\wvs\wvs.exe"
  Delete "$INSTDIR\wvs\COPYING.LIB"
  Delete "$INSTDIR\wvs\liesmich.txt"
  Delete "$INSTDIR\wvs\COPYING.LIB"
  Delete "$INSTDIR\wvs\nonAutoStart.vbs"
  Delete "$INSTDIR\wvs\MM_Auto.vbs"
  Delete "$INSTDIR\wvs\netClient.zip"
  Delete "$INSTDIR\wvs_uninstall.exe"
    
  Delete "$INSTDIR\wvs\mappings\cp936"
  Delete "$INSTDIR\wvs\mappings\cp949"
  Delete "$INSTDIR\wvs\mappings\cp950"
  Delete "$INSTDIR\wvs\mappings\cp850"
  Delete "$INSTDIR\wvs\mappings\cp1250"
  Delete "$INSTDIR\wvs\mappings\cp1251"
  Delete "$INSTDIR\wvs\mappings\8859-9"
  Delete "$INSTDIR\wvs\mappings\8859-8"
  Delete "$INSTDIR\wvs\mappings\8859-7"
  Delete "$INSTDIR\wvs\mappings\8859-6"
  Delete "$INSTDIR\wvs\mappings\8859-5"
  Delete "$INSTDIR\wvs\mappings\8859-4"
  Delete "$INSTDIR\wvs\mappings\8859-3"
  Delete "$INSTDIR\wvs\mappings\8859-2"
  Delete "$INSTDIR\wvs\mappings\8859-16"
  Delete "$INSTDIR\wvs\mappings\8859-15"
  Delete "$INSTDIR\wvs\mappings\8859-14"
  Delete "$INSTDIR\wvs\mappings\8859-13"
  Delete "$INSTDIR\wvs\mappings\8859-11"
  Delete "$INSTDIR\wvs\mappings\8859-10"
  Delete "$INSTDIR\wvs\mappings\8859-1"
  
  Delete "$INSTDIR\wvs\fonts\tga\smallfonts7x7.tga"
  Delete "$INSTDIR\wvs\fonts\tga\interdimensional16.tga"
  Delete "$INSTDIR\wvs\fonts\tga\dotumche24x24.tga"
  Delete "$INSTDIR\wvs\fonts\tga\couriernewce8.tga"
  Delete "$INSTDIR\wvs\fonts\tga\comicsansms7x8.tga"
  Delete "$INSTDIR\wvs\fonts\tga\arial.tga"
  Delete "$INSTDIR\wvs\fonts\bdf\wenquanyi_9pt.bdf"
  Delete "$INSTDIR\wvs\fonts\bdf\snap.bdf"
  Delete "$INSTDIR\wvs\fonts\bdf\rought18.bdf"
  Delete "$INSTDIR\wvs\fonts\bdf\b16_b.bdf"
  Delete "$INSTDIR\wvs\fonts\bdf\5x7.bdf"
  Delete "$INSTDIR\wvs\fonts\bdf\helvr12.bdf"
 
  Delete "$INSTDIR\wvs\UDP_CopyToWinampPlugins\TCP_CopyToWinampPlugins\vis_wamp.dll"
  Delete "$INSTDIR\wvs\UDP_CopyToWinampPlugins\TCP_CopyToWinampPlugins\gen_wamp.dll"
  Delete "$INSTDIR\wvs\UDP_CopyToWinampPlugins\TCP_CopyToWinampPlugins\tcp_readme.txt"
  
  Delete "$INSTDIR\wvs\CopyToWinampPlugins\vis_wamp.dll"
  Delete "$INSTDIR\wvs\CopyToWinampPlugins\gen_wamp.dll"
  
  Delete "$INSTDIR\wvs\CopyToMediaMonkeyPlugins\vis_wamp.dll"
  Delete "$INSTDIR\wvs\CopyToMediaMonkeyPlugins\gen_wamp.dll"
  
  Delete "$INSTDIR\wvs\CopyToAimpPlugins\vis_wvs.svp"
  Delete "$INSTDIR\wvs\CopyToAimpPlugins\gen_wamp.dll"

;  Delete "$INSTDIR\wvs\CopyToXMPlay\vis_wvs.svp"
;  Delete "$INSTDIR\wvs\CopyToXMPlay\xmp-wvs.dll"
  
  Delete "$INSTDIR\wvs\vis_wamp.dll"
  Delete "$INSTDIR\wvs\gen_wamp.dll"
  Delete "$INSTDIR\wvs\vis_wvs.svp"
    
  Delete "$INSTDIR\wvs\CopyToAmsnScripts_0.97rc1\remote.tcl"
  
  Delete "$INSTDIR\wvs\images\msn1.tga"
  Delete "$INSTDIR\wvs\images\msn2.tga"
  Delete "$INSTDIR\wvs\images\mail.tga"

  Delete "$INSTDIR\wvs\libmylcd\COPYING.LIB"
  Delete "$INSTDIR\wvs\libmylcd\mylcd.h"
  Delete "$INSTDIR\wvs\libmylcd\mylcdtypes.h"
  Delete "$INSTDIR\wvs\libmylcd\mylcdconfig.h"
  Delete "$INSTDIR\wvs\libmylcd\libmylcd.a"
  Delete "$INSTDIR\wvs\libmylcd\libmylcdstatic.a"
  Delete "$INSTDIR\wvs\libmylcd\mylcd.lib"
  
  Delete "$INSTDIR\hook.dll"
  Delete "$INSTDIR\mylcd.dll"
  Delete "$INSTDIR\xmp-wvs.dll"
  Delete "$INSTDIR\xmp-wvsclient.dll"
  Delete "$INSTDIR\vis_wvs.svp"
  
  RMDir "$INSTDIR\wvs\mappings"
  RMDir "$INSTDIR\wvs\fonts\tga"
  RMDir "$INSTDIR\wvs\fonts\bdf"
  RMDir "$INSTDIR\wvs\fonts"
  RMDir "$INSTDIR\wvs\CopyToAmsnScripts_0.97rc1"
  RMDir "$INSTDIR\wvs\CopyToWinampPlugins\TCP_CopyToWinampPlugins"
  RMDir "$INSTDIR\wvs\CopyToWinampPlugins"
  RMDir "$INSTDIR\wvs\CopyToMediaMonkeyPlugins"
  RMDir "$INSTDIR\wvs\CopyToAimpPlugins"
  RMDir "$INSTDIR\wvs\images"
  
  RMDir "$INSTDIR\wvs\libmylcd"
  RMDir "$INSTDIR\wvs"

  Delete "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\Website.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\WVS.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\readme.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\changelog.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\Client example (SDK).lnk"
  Delete "$DESKTOP\WVS.lnk"
  RMDir "$SMPROGRAMS\$ICONS_GROUP"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKLM SOFTWARE\WVS
    
  SetAutoClose true
SectionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) has been successfully removed."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd


