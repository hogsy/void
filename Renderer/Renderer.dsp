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
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\Renderer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /D "RENDERER" /D "DYNAMIC_GL" /D "NDEBUG" /D "WIN32" /D "_WINDOWS_RENDERER" /D "RENDERER_EXPORTS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib winmm.lib ole32.lib advapi32.lib ijl11.lib ..\Debug\vfs.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\vrender.dll"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "Renderer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Renderer"
# PROP BASE Intermediate_Dir "Renderer"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\Renderer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /WX /GX /Zd /Od /I "..\Shared" /D "DYNAMIC_GL" /D "_DEBUG" /D "RENDERER" /D "_WIN32" /D "WIN32" /D "_WINDOWS_RENDERER" /D "RENDERER_EXPORTS" /YX"Standard.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ddraw.lib dxguid.lib d3dxd.lib d3dim.lib winmm.lib ole32.lib advapi32.lib ijl11.lib ..\Debug\vfs.lib /nologo /subsystem:windows /dll /profile /map:"..\vrender.map" /debug /machine:I386 /out:"..\vrender.dll" /libpath:"..\Shared\Libraries"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "Renderer - Win32 Release"
# Name "Renderer - Win32 Debug"
# Begin Group "Renderer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ren_beam.cpp
# End Source File
# Begin Source File

SOURCE=.\Ren_beam.h
# End Source File
# Begin Source File

SOURCE=.\Ren_cache.cpp
# End Source File
# Begin Source File

SOURCE=.\Ren_cache.h
# End Source File
# Begin Source File

SOURCE=.\Ren_exp.cpp
# End Source File
# Begin Source File

SOURCE=.\Ren_exp.h
# End Source File
# Begin Source File

SOURCE=.\Ren_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Ren_main.h
# End Source File
# End Group
# Begin Group "Console"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Con_cmds.cpp
# End Source File
# Begin Source File

SOURCE=.\Con_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Con_main.h
# End Source File
# End Group
# Begin Group "Textures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Tex_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Tex_image.cpp
# End Source File
# Begin Source File

SOURCE=.\Tex_image.h
# End Source File
# Begin Source File

SOURCE=.\Tex_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Tex_main.h
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
# Begin Group "Other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Ijl\ijl.h
# End Source File
# End Group
# Begin Group "D3d"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Mssdk\Include\d3dx.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\d3dxcore.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\d3dxerr.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\d3dxmath.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\d3dxmath.inl
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\d3dxshapes.h
# End Source File
# Begin Source File

SOURCE=..\..\Mssdk\Include\d3dxsprite.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\Bsp_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Cl_base.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_camera.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_cvar.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_defs.h
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

SOURCE=..\Shared\Com_trace.h
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

SOURCE=..\Shared\Com_world.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Game_base.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Game_defs.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
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

SOURCE=.\Dll_main.cpp
# End Source File
# End Group
# Begin Group "Rast"

# PROP Default_Filter ""
# Begin Group "OpenGL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gl.h
# End Source File
# Begin Source File

SOURCE=.\gl_driver.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_rast.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_rast.h
# End Source File
# End Group
# Begin Group "None"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Rast_none.cpp
# End Source File
# Begin Source File

SOURCE=.\Rast_none.h
# End Source File
# End Group
# Begin Group "D3DX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Rast_d3dx.cpp
# End Source File
# Begin Source File

SOURCE=.\Rast_d3dx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Rast_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Rast_main.h
# End Source File
# End Group
# Begin Group "Client"

# PROP Default_Filter ""
# Begin Group "Image"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Img_entry.cpp
# End Source File
# Begin Source File

SOURCE=.\Img_entry.h
# End Source File
# Begin Source File

SOURCE=.\Img_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Img_main.h
# End Source File
# End Group
# Begin Group "Models"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mdl_entry.cpp
# End Source File
# Begin Source File

SOURCE=.\Mdl_entry.h
# End Source File
# Begin Source File

SOURCE=.\Mdl_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Mdl_main.h
# End Source File
# Begin Source File

SOURCE=.\Mdl_md2.cpp
# End Source File
# Begin Source File

SOURCE=.\Mdl_md2.h
# End Source File
# Begin Source File

SOURCE=.\Mdl_sp2.cpp
# End Source File
# Begin Source File

SOURCE=.\Mdl_sp2.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Client.h
# End Source File
# End Group
# Begin Group "Shader"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shader.cpp
# End Source File
# Begin Source File

SOURCE=.\Shader.h
# End Source File
# Begin Source File

SOURCE=.\ShaderManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ShaderManager.h
# End Source File
# End Group
# Begin Group "Hud"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Hud_hdr.h
# End Source File
# Begin Source File

SOURCE=.\Hud_main.cpp
# End Source File
# Begin Source File

SOURCE=.\Hud_main.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\plan\js.plan
# End Source File
# Begin Source File

SOURCE=.\Standard.h
# End Source File
# End Target
# End Project
