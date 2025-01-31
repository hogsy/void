# Microsoft Developer Studio Project File - Name="FileSystem" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=FileSystem - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FileSystem.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FileSystem.mak" CFG="FileSystem - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FileSystem - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FileSystem - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FileSystem - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Debug\"
# PROP Intermediate_Dir "..\Debug\FileSystem"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FILESYSTEM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GX /O2 /I "..\Shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FILESYSTEM_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib shell32.lib /nologo /dll /machine:I386 /out:"..\vfs.dll"

!ELSEIF  "$(CFG)" == "FileSystem - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\FileSystem"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FILESYSTEM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\Shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FILESYSTEM_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib shell32.lib /nologo /dll /map:"..\vfs.map" /debug /machine:I386 /out:"..\vfs.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "FileSystem - Win32 Release"
# Name "FileSystem - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Fs_cfiles.cpp
# End Source File
# Begin Source File

SOURCE=.\Fs_filebuf.cpp
# End Source File
# Begin Source File

SOURCE=.\Fs_filestream.cpp
# End Source File
# Begin Source File

SOURCE=.\Fs_filesys.cpp
# End Source File
# Begin Source File

SOURCE=.\Fs_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Fs_pakfile.cpp
# End Source File
# Begin Source File

SOURCE=.\Fs_zipfile.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Fs_filesys.h
# End Source File
# Begin Source File

SOURCE=.\Fs_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Fs_pakfile.h
# End Source File
# Begin Source File

SOURCE=.\Fs_readers.h
# End Source File
# Begin Source File

SOURCE=.\Fs_zipfile.h
# End Source File
# End Group
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Com_defs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_file.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_file.h
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

SOURCE=..\Shared\Com_util.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_util.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_hunkmem.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\I_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_filesystem.h
# End Source File
# End Target
# End Project
