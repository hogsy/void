# Microsoft Developer Studio Project File - Name="Renderer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Renderer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Renderer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Renderer.mak" CFG="Renderer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Renderer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Renderer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Renderer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Debug\Out\Renderer"
# PROP Intermediate_Dir "..\Debug\Temp\Renderer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /D "NDEBUG" /D "WIN32" /D "_WINDOWS_RENDERER" /D "RENDERER_EXPORTS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib glu32.lib ole32.lib kernel32.lib user32.lib gdi32.lib shell32.lib advapi32.lib ijl11.lib ..\Debug\Out\FileSystem\vfs.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\vrender.dll"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "Renderer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Renderer"
# PROP BASE Intermediate_Dir "Renderer"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\Out\Renderer"
# PROP Intermediate_Dir "..\Debug\Temp\Renderer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /WX /GX /ZI /Od /D "DYNAMIC_GL" /D "_DEBUG" /D "RENDERER" /D "_WIN32" /D "WIN32" /D "_WINDOWS_RENDERER" /D "RENDERER_EXPORTS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib winmm.lib ole32.lib advapi32.lib ijl11.lib ..\Debug\Out\FileSystem\vfs.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\vrender.dll"
# SUBTRACT LINK32 /profile /map

!ENDIF 

# Begin Target

# Name "Renderer - Win32 Release"
# Name "Renderer - Win32 Debug"
# Begin Group "Renderer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Ren_beam.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_beam.h
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_cache.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_cache.h
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_exp.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_exp.h
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_main.h
# End Source File
# End Group
# Begin Group "OpenGL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\gl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\gl.h
# End Source File
# Begin Source File

SOURCE=.\Source\Gl_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Gl_main.h
# End Source File
# End Group
# Begin Group "Console"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Con_cmds.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Con_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Con_main.h
# End Source File
# End Group
# Begin Group "Textures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Tex_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Source\Tex_image.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Tex_image.h
# End Source File
# Begin Source File

SOURCE=.\Source\Tex_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Tex_main.h
# End Source File
# End Group
# Begin Group "Light"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Light_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Light_main.h
# End Source File
# End Group
# Begin Group "Models"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Mdl_cache.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Mdl_cache.h
# End Source File
# Begin Source File

SOURCE=.\Source\Mdl_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Mdl_main.h
# End Source File
# End Group
# Begin Group "Hud"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Hud_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Hud_main.h
# End Source File
# End Group
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Group "3dfx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\3dfx\3DFX.H
# End Source File
# Begin Source File

SOURCE=..\Shared\3dfx\glide.h
# End Source File
# Begin Source File

SOURCE=..\Shared\3dfx\glidesys.h
# End Source File
# Begin Source File

SOURCE=..\Shared\3dfx\glideutl.h
# End Source File
# Begin Source File

SOURCE=.\Source\Ren_sky.h
# End Source File
# Begin Source File

SOURCE=..\Shared\3dfx\SST1VID.H
# End Source File
# End Group
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

SOURCE=..\Shared\Com_mem.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_mem.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_registry.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_registry.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_util.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_util.h
# End Source File
# Begin Source File

SOURCE=..\Shared\world.h
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
# Begin Group "Dll"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Dll_main.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\Com_defs.h
# End Source File
# Begin Source File

SOURCE=..\plan\js.plan
# End Source File
# Begin Source File

SOURCE=.\Source\Standard.h
# End Source File
# End Target
# End Project
