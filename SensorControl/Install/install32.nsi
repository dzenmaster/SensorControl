!include LogicLib.nsh
!include "x64.nsh"
!include "MUI.nsh"

icon ..\icon1.ico

Name "SensorControl"

# define the name of the installer
Outfile "SensorControl_32.exe"
 
# define the directory to install to, the desktop in this case as specified  
# by the predefined $DESKTOP variable

InstallDir "$PROGRAMFILES\SensorControl"
 
# default section
Section
 
# define the output path for this file
SetOutPath $INSTDIR

CreateDirectory "$PROGRAMFILES\SensorControl"
 
;File /r win32
File /r ..\win32\Release\*.*

;create desktop shortcut
CreateShortCut "$DESKTOP\SensorControl 32bit.lnk" "$INSTDIR\SensorControl.exe" ""
 
SectionEnd