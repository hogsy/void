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
# PROP Output_Dir "..\Debug\"
# PROP Intermediate_Dir "..\Debug\Exe"
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib winmm.lib dxguid.lib dinput.lib dsound.lib Ws2_32.lib ..\Debug\vrender.lib ..\Debug\vfs.lib ..\Debug\vnet.lib /nologo /subsystem:windows /machine:I386 /out:"..\Void.exe"
# SUBTRACT LINK32 /profile /map /debug

!ELSEIF  "$(CFG)" == "Void - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Void___W"
# PROP BASE Intermediate_Dir "Void___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\"
# PROP Intermediate_Dir "..\Debug\Exe"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /WX /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_VOID_EXE_" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib winmm.lib dxguid.lib dinput.lib dsound.lib Ws2_32.lib ..\Debug\vrender.lib ..\Debug\vfs.lib ..\Debug\vnet.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\Void.exe"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "Void - Win32 Release"
# Name "Void - Win32 Debug"
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Snd_buf.cpp
# End Source File
# Begin Source File

SOURCE=.\Snd_buf.h
# End Source File
# Begin Source File

SOURCE=.\Snd_chan.cpp
# End Source File
# Begin Source File

SOURCE=.\Snd_chan.h
# End Source File
# Begin Source File

SOURCE=.\Snd_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Snd_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Snd_main.h
# End Source File
# Begin Source File

SOURCE=.\Snd_wave.cpp
# End Source File
# Begin Source File

SOURCE=.\Snd_wave.h
# End Source File
# End Group
# Begin Group "System"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sys_cons.cpp
# End Source File
# Begin Source File

SOURCE=.\Sys_cons.h
# End Source File
# Begin Source File

SOURCE=.\Sys_exp.h
# End Source File
# Begin Source File

SOURCE=.\Sys_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Sys_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Sys_main.h
# End Source File
# Begin Source File

SOURCE=.\Sys_time.cpp
# End Source File
# Begin Source File

SOURCE=.\Sys_time.h
# End Source File
# Begin Source File

SOURCE=.\Sys_win.cpp
# End Source File
# End Group
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Bsp_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_buffer.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_buffer.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_cvar.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_defs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_hunk.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_hunk.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_keys.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_mem.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_mem.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_parms.cpp
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

SOURCE=..\Shared\Com_vector.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_vector.h
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

SOURCE=.\Cl_cmds.cpp
# End Source File
# Begin Source File

SOURCE=.\Cl_cmds.h
# End Source File
# Begin Source File

SOURCE=.\Cl_collision.cpp
# End Source File
# Begin Source File

SOURCE=.\Cl_collision.h
# End Source File
# Begin Source File

SOURCE=.\Cl_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Cl_main.h
# End Source File
# Begin Source File

SOURCE=.\Cl_move.cpp
# End Source File
# Begin Source File

SOURCE=.\Cl_net.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Clgame_defs.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\resources.h
# End Source File
# Begin Source File

SOURCE=.\resources.rc
# End Source File
# End Group
# Begin Group "Music"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mus_cd.cpp
# End Source File
# Begin Source File

SOURCE=.\Mus_cd.h
# End Source File
# Begin Source File

SOURCE=.\Mus_dm.cpp
# End Source File
# Begin Source File

SOURCE=.\Mus_dm.h
# End Source File
# Begin Source File

SOURCE=.\Mus_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Mus_main.h
# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Game_ents.cpp
# End Source File
# Begin Source File

SOURCE=.\Game_ents.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_game.h
# End Source File
# Begin Source File

SOURCE=.\Sv_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Sv_main.h
# End Source File
# Begin Source File

SOURCE=.\Sv_net.cpp
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\In_defs.h
# End Source File
# Begin Source File

SOURCE=.\In_kb.cpp
# End Source File
# Begin Source File

SOURCE=.\In_kb.h
# End Source File
# Begin Source File

SOURCE=.\In_main.cpp
# End Source File
# Begin Source File

SOURCE=.\In_main.h
# End Source File
# Begin Source File

SOURCE=.\In_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\In_mouse.h
# End Source File
# Begin Source File

SOURCE=.\In_state.cpp
# End Source File
# Begin Source File

SOURCE=.\In_state.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\I_client.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_clientRenderer.h
# End Source File
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

SOURCE=..\Shared\I_netclient.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_renderer.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_void.h
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Net_client.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Net_defs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Net_protocol.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Net_server.h
# End Source File
# End Group
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Mssdk\Include\BaseTsd.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\Guiddef.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\PropIdl.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\Tvout.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\WinEFS.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\Game_defs.h
# End Source File
# Begin Source File

SOURCE=.\todo
# End Source File
# Begin Source File

SOURCE=.\worklog
# End Source File
# End Target
# End Project
