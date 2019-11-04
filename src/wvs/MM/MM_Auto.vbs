
' Use this script only if you are consistently experiencing a problem autostarting WVS with MM.
' Do not use this script if WVS autostarts normally.



Sub OnStartup
  Dim WShell, Command, result
  Set WShell = CreateObject("WScript.Shell")
  Command = "cmd /C CD " &Chr(34) &SDB.ApplicationPath &"Plugins\wvs" &Chr(34) &" & wvs.exe"
  result = WShell.Run (Command,0,0)
end Sub

' by Peke






