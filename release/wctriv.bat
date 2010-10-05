@echo off

REM  WCTRIV.BAT - Batch file for running Tournament Trivia on Wildcat WinServer
REM  You will need to fix directories as explained below.

REM  Please note:
REM  - Only use this batch file if you are using Wildcat Winserver.
REM  - Be sure that the door is able to find Winserver's DOOR32.DLL; please
REM    see README.TXT for more info.
REM  - Do not add any other lines to this batch file!!! The door will/MUST 
REM    switch to its own directory automatically.  Do NOT add a cd\ line to
REM    this batch file to switch to the door's directory!

REM  The example here assumes c:\doors\trivia is your Trivia directory, and
REM  also assumes c:\doors\node# is where your BBS puts its dropfiles.  You'll
REM  need to change the line below to reflect your own system's directories.

REM  *** Mofiy the directories in the line below as noted above ***
c:\doors\trivia\triv32.exe -n %wcnodeid% -d c:\doors\node%wcnodeid%

REM  End of batch file.