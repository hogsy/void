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
# PROP Output_Dir "..\Debug\Out\Exe"
# PROP Intermediate_Dir "..\Debug\Temp\Exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_EXE" /D "_VOID_EXE_" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib winmm.lib dxguid.lib dinput.lib dsound.lib Ws2_32.lib ...\Debug\Out\FileSystem\vfs.lib ..\Debug\Out\Renderer\vrender.lib /nologo /subsystem:windows /profile /machine:I386 /out:"..\Void.exe"
# SUBTRACT LINK32 /map /debug

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
# ADD CPP /nologo /W3 /WX /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_VOID_EXE_" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib winmm.lib dxguid.lib dinput.lib dsound.lib Ws2_32.lib ..\Debug\Out\Renderer\vrender.lib ..\Debug\Out\FileSystem\vfs.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\Void.exe"
# SUBTRACT LINK32 /pdb:none

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

SOURCE=.\Source\Snd_defs.h
# End Source File
# Begin Source File

SOURCE=.\Source\Snd_hdr.h
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

SOURCE=.\Source\Sys_exp.h
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
# Begin Source File

SOURCE=.\Source\Sys_win.cpp
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

SOURCE=..\Shared\Com_cvar.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_hunk.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_hunk.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_mem.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_mem.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_parms.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_util.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_util.h
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

SOURCE=.\Source\Cl_cmds.h
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

SOURCE=..\..\DevStudio\VC98\Include\BASETSD.H
# End Source File
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

SOURCE=..\Shared\Fmod\fmod.h
# End Source File
# Begin Source File

SOURCE=.\Source\Mus_cd.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Mus_cd.h
# End Source File
# Begin Source File

SOURCE=.\Source\Mus_dm.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Mus_dm.h
# End Source File
# Begin Source File

SOURCE=.\Source\Mus_fmod.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Mus_fmod.h
# End Source File
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
# Begin Source File

SOURCE=.\Source\In_state.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\In_state.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\I_console.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_filesystem.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_hud.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_hunkmem.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_renderer.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_void.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\Com_defs.h
# End Source File
# Begin Source File

SOURCE=.\Source\Sys_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Source\todo
# End Source File
# Begin Source File

SOURCE=.\Source\worklog
# End Source File
# End Target
# End Project
