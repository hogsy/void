# Microsoft Developer Studio Project File - Name="Network" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Network - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Network.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Network.mak" CFG="Network - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Network - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Network - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Network - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\Network"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Network - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\Network"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /WX /Gm /GX /ZI /Od /I "..\Shared" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Debug\vnet.lib"

!ENDIF 

# Begin Target

# Name "Network - Win32 Release"
# Name "Network - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Net_chan.cpp
# End Source File
# Begin Source File

SOURCE=.\Net_clchan.cpp
# End Source File
# Begin Source File

SOURCE=.\Net_client.cpp
# End Source File
# Begin Source File

SOURCE=.\Net_server.cpp
# End Source File
# Begin Source File

SOURCE=.\Net_sock.cpp
# End Source File
# Begin Source File

SOURCE=.\Net_util.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Net_chan.h
# End Source File
# Begin Source File

SOURCE=.\Net_clchan.h
# End Source File
# Begin Source File

SOURCE=.\Net_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Net_sock.h
# End Source File
# Begin Source File

SOURCE=.\Net_util.h
# End Source File
# End Group
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\3dmath.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_buffer.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_buffer.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_defs.h
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
# End Target
# End Project
