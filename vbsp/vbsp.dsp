# Microsoft Developer Studio Project File - Name="vbsp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vbsp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vbsp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vbsp.mak" CFG="vbsp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vbsp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vbsp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vbsp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "vbsp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\Out\vbsp"
# PROP Intermediate_Dir "..\Debug\Temp\vbsp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\vbsp.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "vbsp - Win32 Release"
# Name "vbsp - Win32 Debug"
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\bsp_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_vector.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_vector.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Shader.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Shader.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ShaderManager.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ShaderManager.h
# End Source File
# Begin Source File

SOURCE=.\source\std_lib.cpp
# End Source File
# Begin Source File

SOURCE=.\source\std_lib.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\source\bsp.cpp
# End Source File
# Begin Source File

SOURCE=.\source\bsp.h
# End Source File
# Begin Source File

SOURCE=.\source\bsp_write.cpp
# End Source File
# Begin Source File

SOURCE=.\source\cam_path.cpp
# End Source File
# Begin Source File

SOURCE=.\source\cam_path.h
# End Source File
# Begin Source File

SOURCE=.\source\csg.cpp
# End Source File
# Begin Source File

SOURCE=.\source\csg.h
# End Source File
# Begin Source File

SOURCE=.\source\entity.cpp
# End Source File
# Begin Source File

SOURCE=.\source\entity.h
# End Source File
# Begin Source File

SOURCE=.\source\map_file.cpp
# End Source File
# Begin Source File

SOURCE=.\source\map_file.h
# End Source File
# Begin Source File

SOURCE=.\source\portal.cpp
# End Source File
# Begin Source File

SOURCE=.\source\portal.h
# End Source File
# Begin Source File

SOURCE=.\source\vbsp.cpp
# End Source File
# Begin Source File

SOURCE=.\source\vbsp.h
# End Source File
# End Target
# End Project
