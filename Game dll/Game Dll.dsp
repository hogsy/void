# Microsoft Developer Studio Project File - Name="Game Dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Game Dll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Game Dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Game Dll.mak" CFG="Game Dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Game Dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Game Dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Game Dll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\GameDll"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAMEDLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAMEDLL_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib /nologo /dll /machine:I386 /out:"..\vgame.dll"

!ELSEIF  "$(CFG)" == "Game Dll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\GameDll"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAMEDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAMEDLL_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib /nologo /dll /debug /machine:I386 /out:"..\vgame.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Game Dll - Win32 Release"
# Name "Game Dll - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Game_ents.cpp
# End Source File
# Begin Source File

SOURCE=.\Game_main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Game.def
# End Source File
# Begin Source File

SOURCE=..\Shared\Game_defs.h
# End Source File
# Begin Source File

SOURCE="..\Void Game\Game_ents.h"
# End Source File
# Begin Source File

SOURCE=.\Game_main.h
# End Source File
# End Group
# Begin Group "Shared"

# PROP Default_Filter ""
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

SOURCE=..\Shared\I_console.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Net_defs.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\I_game.h
# End Source File
# End Target
# End Project
