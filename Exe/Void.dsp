# Microsoft Developer Studio Project File - Name="Void" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Void - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Void.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Void.mak" CFG="Void - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Void - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Void - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Void - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\Out\Exe"
# PROP Intermediate_Dir "..\Release\Temp\Exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_EXE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib Ws2_32.lib winmm.lib dxguid.lib dinput.lib dsound.lib version.lib fmodvc.lib /nologo /subsystem:windows /profile /map:"..\Void.map" /machine:I386 /out:"D:\Void\Void.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "Void - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Void___W"
# PROP BASE Intermediate_Dir "Void___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\Out\Exe"
# PROP Intermediate_Dir "..\Debug\Temp\Exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gz /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib winmm.lib dxguid.lib dinput.lib dsound.lib fmodvc.lib Ws2_32.lib /nologo /subsystem:windows /incremental:no /map:"D:\Void\Void.map" /debug /debugtype:both /machine:I386 /out:"..\Void.exe"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "Void - Win32 Release"
# Name "Void - Win32 Debug"
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Snd_buf.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Snd_buf.h
# End Source File
# Begin Source File

SOURCE=.\Source\Snd_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Snd_main.h
# End Source File
# Begin Source File

SOURCE=.\Source\Snd_wave.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Snd_wave.h
# End Source File
# End Group
# Begin Group "System"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Sys_cons.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Sys_cons.h
# End Source File
# Begin Source File

SOURCE=.\Source\Sys_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Sys_main.h
# End Source File
# Begin Source File

SOURCE=.\Source\Sys_time.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Sys_time.h
# End Source File
# End Group
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\3dmath.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\3dmath.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Bsp_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Clip.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\clip.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Fmod\fmod.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_console.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_hud.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_renderer.h
# End Source File
# Begin Source File

SOURCE=..\Shared\World.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\world.h
# End Source File
# End Group
# Begin Group "Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Cl_cmds.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Cl_collision.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Cl_collision.h
# End Source File
# Begin Source File

SOURCE=.\Source\Cl_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Cl_main.h
# End Source File
# Begin Source File

SOURCE=.\Source\Cl_move.cpp
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\Source\resources.h
# End Source File
# Begin Source File

SOURCE=.\Source\resources.rc
# End Source File
# End Group
# Begin Group "Music"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Mus_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Mus_main.h
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Net_defs.h
# End Source File
# Begin Source File

SOURCE=.\Source\Net_sock.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Net_sock.h
# End Source File
# Begin Source File

SOURCE=.\Source\Net_util.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Net_util.h
# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Sv_client.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Sv_client.h
# End Source File
# Begin Source File

SOURCE=.\Source\Sv_ents.h
# End Source File
# Begin Source File

SOURCE=.\Source\Sv_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Sv_main.h
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\In_defs.h
# End Source File
# Begin Source File

SOURCE=.\Source\In_kb.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\In_kb.h
# End Source File
# Begin Source File

SOURCE=.\Source\In_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\In_main.h
# End Source File
# Begin Source File

SOURCE=.\Source\In_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\In_mouse.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Com_cvar.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_cvar.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_file.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_filesys.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_filesys.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_mem.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_mem.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_pakfile.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_pakfile.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_zipfile.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_zipfile.h
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\q3netinfo
# End Source File
# Begin Source File

SOURCE=.\Source\todo
# End Source File
# Begin Source File

SOURCE=.\Source\worklog
# End Source File
# End Group
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Com_buffer.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_buffer.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_list.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_queue.h
# End Source File
# Begin Source File

SOURCE=.\Source\Util_sys.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Util_sys.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\Com_defs.h
# End Source File
# Begin Source File

SOURCE=.\Source\Sys_hdr.h
# End Source File
# End Target
# End Project
