#include "Standard.h"
#include "Com_parms.h"
#include "Con_main.h"
#include "Tex_image.h"
#include "Com_util.h"

/*
==========================================
Extension of the Renderer Console to manage
initialization/registration of global commands/objects
==========================================
*/
enum
{
	CMD_TGASHOT = 0,
	CMD_PCXSHOT = 1
};


CVar * g_varWndX;        //Windowed X pos
CVar * g_varWndY;        //Windowed Y pos
CVar * g_varFullbright;
CVar * g_varFov;
CVar * g_varMultiTexture;
CVar * g_varVidSynch;
CVar * g_var32BitTextures;
CVar * g_varBeamTolerance;
CVar * g_varD3DXShift;
CVar * g_varGLExtensions;
CVar * g_varGLDriver;


//======================================================================================
//Command Handling routines
//======================================================================================

/*
======================================
Take a screen shot, write it to disk
======================================
*/
void ScreenShot(EImageFileFormat type)
{
	char checkname[260];

	//find a file name to save it to 
	sprintf(checkname, "%s\\%s\\", GetCurPath(), "Shots");
	Util::ConfirmDir(checkname);


	FILE *fp =0;
	char  shotname[80]; 

	if(type== FORMAT_PCX)
		strcpy(shotname,"void00.pcx");
	else
		strcpy(shotname,"void00.tga");

	for (int i=0 ; i<=99 ; i++) 
	{
		shotname[4] = i/10 + '0';
		shotname[5] = i%10 + '0';
		sprintf(checkname,"%s/Shots/%s", GetCurPath(), shotname);

		fp = fopen(checkname,"rb");

		if (!fp)
			break;
		fclose(fp);

	}

	if (i== 100) 
	{
		ComPrintf("Too many Screenshots in Shots folder. Please remove some\n");
		return;
	}

	//Got file name, Now actually take the shot and write it
	int  width  = g_rInfo.width;
	int  height = g_rInfo.height;
	byte * data = (byte*)g_pHunkManager->HunkAlloc(width * height * 4);

	if (data == NULL) 
	{
		Error("ScreenShot:No mem to capture screendata");
		return;
	}
	g_pRast->ScreenShot(data);

	CImageWriter imageWriter(width,height,data);
	imageWriter.Write(checkname,type);
	g_pHunkManager->HunkFree(data);
}

/*
==========================================
Handle Global Commands
==========================================
*/
void CRConsole::HandleCommand(int cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_TGASHOT:
		{
			ScreenShot(FORMAT_TGA);
			break;
		}
	case CMD_PCXSHOT:
		{
			ScreenShot(FORMAT_PCX);
			break;
		}
	}
}

/*
======================================================================================
CVar Handling funcs
======================================================================================
*/
/*
=======================================
switch fullbright (light) rendering
=======================================
*/
bool CVar_FullBright(int val)
{
	return true;
}

/*
=======================================
toggle multitexture/multipass rendering
=======================================
*/
bool CVar_MultiTexture(int val)
{
/*
	if(val)
	{
		if (!(g_rInfo.rflags & RFLAG_MULTITEXTURE))
			ComPrintf("Your video card does not support ARB multitexturing.\n");
		return false;
	}
*/
	return true;
}

/*
=======================================
switch fullbright (light) rendering
=======================================
*/
bool CVar_Fov(int val)
{
	if (val>=10 && val<= 170)
		return true;
	return false;
}

/*
=======================================
toggle vid synch
=======================================
*/
bool CVar_VidSynch(int val)
{
	if (val)
		g_pRast->SetVidSynch(1);
	else
		g_pRast->SetVidSynch(0);
	return true;
}


/*
=======================================
16 / 32 bit textures
=======================================
*/
bool CVar_32BitTextures(int val)
{
	ComPrintf("Change will take effect on next level load.\n");
	return true;
}


/*
=======================================
set how much the console will fade in
=======================================
*/
bool CVar_ConAlpha(int val)
{
	if (val<100 || val>255)
		return false;
	return true;
}

/*
=======================================
tolerance for caching polys as zfill/beamtree or zbuffer
=======================================
*/
bool CVar_BeamTolerance(int val)
{
	if (val<0)
		return false;
	return true;
}

bool CVar_D3DXShift(int val)
{	return true;
}

bool CVar_GLDriver(int val)
{
	return true;
}


//======================================================================================
//======================================================================================

/*
==========================================
Handle Cvars
==========================================
*/
bool CRConsole::HandleCVar(const CVar * cvar, const CStringVal &strVal)
{
	if(cvar == g_varFullbright)
		return CVar_FullBright(strVal.IntVal());

	else if(cvar == g_varFov)
		return CVar_Fov(strVal.IntVal());

	else if(cvar == g_varMultiTexture)
		return CVar_MultiTexture(strVal.IntVal());

	else if(cvar == g_varVidSynch)
		return CVar_VidSynch(strVal.IntVal());

	else if(cvar == g_var32BitTextures)
		return CVar_32BitTextures(strVal.IntVal());

	else if(cvar == m_varConAlpha)
		return CVar_ConAlpha(strVal.IntVal());

	else if(cvar == g_varBeamTolerance)
		return CVar_BeamTolerance(strVal.IntVal());

	else if(cvar == g_varD3DXShift)
		return CVar_D3DXShift(strVal.IntVal());

	else if(cvar == g_varGLDriver)
		return CVar_GLDriver(strVal.IntVal());

	return false;
}

/*
==========================================
Register Cvars and Commands, called from
Console constructor
==========================================
*/
void CRConsole::RegisterConObjects()
{
	I_Console * pConsole = I_Console::GetConsole();

	pConsole->RegisterCommand("tga_shot",CMD_TGASHOT,this);
	pConsole->RegisterCommand("pcx_shot",CMD_PCXSHOT,this);

	g_varWndX = pConsole->RegisterCVar("r_wndx","80",CVAR_INT,CVAR_ARCHIVE,this);
	g_varWndY = pConsole->RegisterCVar("r_wndy","40",CVAR_INT,CVAR_ARCHIVE,this);

	g_varFullbright		= pConsole->RegisterCVar("r_fullbright","0",CVAR_INT,CVAR_ARCHIVE,this);
	g_varFov			= pConsole->RegisterCVar("r_fov","90", CVAR_INT,CVAR_ARCHIVE,this);
	g_varMultiTexture	= pConsole->RegisterCVar("r_multitexture","1", CVAR_INT,CVAR_ARCHIVE,this);
	g_varVidSynch		= pConsole->RegisterCVar("r_vidsynch","0",CVAR_INT, CVAR_ARCHIVE,this);
	g_var32BitTextures	= pConsole->RegisterCVar("r_32bittextures","1", CVAR_BOOL,CVAR_ARCHIVE,this);
	g_varBeamTolerance	= pConsole->RegisterCVar("r_beamtolerance","25", CVAR_FLOAT,CVAR_ARCHIVE,this);
	g_varD3DXShift		= pConsole->RegisterCVar("r_d3dx_text_shift", "0", CVAR_INT, CVAR_ARCHIVE,this);
	g_varGLExtensions	= pConsole->RegisterCVar("r_glExts", "None", CVAR_STRING, CVAR_READONLY, this);
	g_varGLDriver		= pConsole->RegisterCVar("r_glDriver", "0", CVAR_INT, CVAR_ARCHIVE, this);
}


