

  Dim WShell, Command, result
  Set WShell = CreateObject("WScript.Shell")
  Command = "cmd /C CD C:\Program Files\Winamp\Plugins\wvs & wvs.exe"
  result = WShell.Run (Command,0,0) 

  Command = "cmd /C CD C:\Program Files\Winamp & winamp.exe"
  result = WShell.Run (Command,0,0) 


  strComputer = "."
  Set objWMIService = GetObject _
  ("winmgmts:\\" & strComputer & "\root\cimv2")

  Set colProcessList = objWMIService.ExecQuery _
    ("Select * from Win32_Process Where Name = 'cmd.exe'")

  For Each objProcess in colProcessList
    objProcess.Terminate()
  next


