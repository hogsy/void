# Microsoft Developer Studio Project File - Name="Devvoid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Devvoid - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Devvoid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Devvoid.mak" CFG="Devvoid - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Devvoid - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Devvoid - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Devvoid - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Devvoid - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug\Devvoid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_VLIGHT_" /D "DEVVOID" /FR /YX /FD /GZ /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\Debug\vfs.lib ijl11.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\Devvoid.exe" /pdbtype:sept /libpath:"..\Shared\Libraries"
# SUBTRACT LINK32 /map

!ENDIF 

# Begin Target

# Name "Devvoid - Win32 Release"
# Name "Devvoid - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Devvoid.cpp
# End Source File
# Begin Source File

SOURCE=.\Devvoid.rc
# End Source File
# Begin Source File

SOURCE=.\DevvoidDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Devvoid.h
# End Source File
# Begin Source File

SOURCE=.\DevvoidDlg.h
# End Source File
# Begin Source File

SOURCE=.\OutputWnd.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Devvoid.ico
# End Source File
# Begin Source File

SOURCE=.\res\Devvoid.rc2
# End Source File
# End Group
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\bsp_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_defs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_fastmath.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_fastmath.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_registry.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_registry.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_release.h
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

SOURCE=..\Shared\Com_world.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Com_world.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\I_filesystem.h
# End Source File
# Begin Source File

SOURCE=..\Renderer\Rast_main.h
# End Source File
# Begin Source File

SOURCE=..\Renderer\Shader.cpp
# End Source File
# Begin Source File

SOURCE=..\Renderer\Shader.h
# End Source File
# Begin Source File

SOURCE=..\Renderer\ShaderManager.cpp
# End Source File
# Begin Source File

SOURCE=..\Renderer\ShaderManager.h
# End Source File
# Begin Source File

SOURCE=..\Renderer\Tex_image.cpp
# End Source File
# Begin Source File

SOURCE=..\Renderer\Tex_image.h
# End Source File
# End Group
# Begin Group "Bsp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Bsp\bsp.cpp
# End Source File
# Begin Source File

SOURCE=.\Bsp\bsp.h
# End Source File
# Begin Source File

SOURCE=.\Bsp\bsp_write.cpp
# End Source File
# Begin Source File

SOURCE=.\Bsp\cam_path.cpp
# End Source File
# Begin Source File

SOURCE=.\Bsp\csg.cpp
# End Source File
# Begin Source File

SOURCE=.\Bsp\entity.cpp
# End Source File
# Begin Source File

SOURCE=.\Bsp\entity.h
# End Source File
# Begin Source File

SOURCE=.\Bsp\map_file.cpp
# End Source File
# Begin Source File

SOURCE=.\Bsp\map_file.h
# End Source File
# Begin Source File

SOURCE=.\Bsp\portal.cpp
# End Source File
# Begin Source File

SOURCE=.\Bsp\portal.h
# End Source File
# Begin Source File

SOURCE=.\Bsp\vbsp.cpp
# End Source File
# End Group
# Begin Group "Light"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Light\light.cpp
# End Source File
# Begin Source File

SOURCE=.\Light\light.h
# End Source File
# Begin Source File

SOURCE=.\Light\lightmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Light\lightmap.h
# End Source File
# Begin Source File

SOURCE=.\Light\vlight.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\Std_lib.h
# End Source File
# End Target
# End Project
