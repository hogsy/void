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

CVar *	g_pFullbright=0;
CVar *	g_pFov=0;
CVar *	g_pMultiTexture=0;
CVar *	g_pVidSynch=0;
CVar *	g_p32BitTextures=0;
CVar *	g_pBeamTolerance=0;
CVar *	g_pD3DXShift=0;	// d3dx text shift value

//======================================================================================
//Command Handling routines
//======================================================================================

/*
======================================
Take a screen shot, write it to disk
======================================
*/
void ScreenShot(const char *name, EImageFileFormat type)
{
	char	checkname[260];

	//find a file name to save it to 
	sprintf(checkname, "%s\\%s\\", GetCurPath(), "Shots");
	Util::ConfirmDir(checkname);

	if (!name)
	{
		int	  i;
		FILE *f;
		char  shotname[80]; 

		if(type== FORMAT_PCX)
			strcpy(shotname,"void00.pcx");
		else
			strcpy(shotname,"void00.tga");

		for (i=0 ; i<=99 ; i++) 
		{
			shotname[4] = i/10 + '0';
			shotname[5] = i%10 + '0';
			sprintf(checkname,"%s\\%s\\%s", GetCurPath(), "Shots", shotname);
			f = fopen(checkname,"rb");
			if (!f)
				break;
			fclose(f);
		}

		if (i==100) 
		{
			ComPrintf("too many screen shots - try deleteing or moving some\n"); 
			return;
		}
	}
	else
	{
		sprintf(checkname, "%s\\%s\\%s", GetCurPath(), "Shots", name);
		if(type == FORMAT_PCX)
			Util::SetDefaultExtension(checkname, "pcx");
		else
			Util::SetDefaultExtension(checkname, "tga");
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
			char fileName[80];
			parms.StringTok(1,fileName,80);
			if(strlen(fileName))
				ScreenShot(fileName,FORMAT_TGA);
			else
				ScreenShot(0,FORMAT_TGA);
			break;
		}
	case CMD_PCXSHOT:
		{
			char fileName[80];
			parms.StringTok(1,fileName,80);
			if(strlen(fileName))
				ScreenShot(fileName,FORMAT_PCX);
			else
				ScreenShot(0,FORMAT_PCX);
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
bool CVar_FullBright(const CVar * var, int val)
{
	//There was no second parm 
	if(val == COM_INVALID_VALUE)
		return false;

	if (val)
		g_rInfo.rflags |= RFLAG_FULLBRIGHT;
	else
		g_rInfo.rflags &= ~RFLAG_FULLBRIGHT;
	return true;
}

/*
=======================================
toggle multitexture/multipass rendering
=======================================
*/
bool CVar_MultiTexture(const CVar * var, int val)
{
	if(val == COM_INVALID_VALUE)
	{
		if (!(g_rInfo.rflags & RFLAG_MULTITEXTURE))
			ComPrintf("Your video card does not support ARB multitexturing.\n");
		return false;
	}
	return true;
}

/*
=======================================
switch fullbright (light) rendering
=======================================
*/
bool CVar_Fov(const CVar * var, int val)
{
	if(val == COM_INVALID_VALUE)
		return false;

	if (val>=10 && val<= 170)
	{
//		g_pRast->ProjectionMode(VRAST_PERSPECTIVE);
		return true;
	}
	return false;
}

/*
=======================================
toggle vid synch
=======================================
*/
bool CVar_VidSynch(const CVar * var, int val)
{
	if(val == COM_INVALID_VALUE)
		return false;

	if (g_rInfo.rflags & RFLAG_SWAP_CONTROL)
	{
		if (val)
			g_pRast->SetVidSynch(1);
		else
			g_pRast->SetVidSynch(0);
		return true;
	}
	ComPrintf("Video card doesnt support VidSynch\n");
	return false;
}


/*
=======================================
16 / 32 bit textures
=======================================
*/
bool CVar_32BitTextures(const CVar * var, int val)
{
	if(val == COM_INVALID_VALUE) 
		return false;

	ComPrintf("Change will take effect on next level load.\n");
	return true;
}


/*
=======================================
set how much the console will fade in
=======================================
*/
bool CVar_ConAlpha(const CVar * var, int val)
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
bool CVar_BeamTolerance(const CVar * var, const CParms &parms)
{
	if (parms.NumTokens() > 1)
	{
		int temp= parms.IntTok(1);
		if (temp<0)
			return false;
		return true;
	}
	return false;
}


/*
=======================================
set how much the console will fade in
=======================================
*/
bool CVar_D3DXShift(const CVar * var, int val)
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
bool CRConsole::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == g_pFullbright)
		return CVar_FullBright((CVar*)cvar,parms.IntTok(1));
	else if(cvar == g_pFov)
		return CVar_Fov((CVar*)cvar,parms.IntTok(1));
	else if(cvar == g_pMultiTexture)
		return CVar_MultiTexture((CVar*)cvar,parms.IntTok(1));
	else if(cvar == g_pVidSynch)
		return CVar_VidSynch((CVar*)cvar,parms.IntTok(1));
	else if(cvar == g_p32BitTextures)
		return CVar_32BitTextures((CVar*)cvar,parms.IntTok(1));
	else if(cvar == (CVarBase*)&m_conAlpha)
		return CVar_ConAlpha(&m_conAlpha,parms.IntTok(1));
	else if(cvar == g_pBeamTolerance)
		return CVar_BeamTolerance((CVar*)cvar,parms);
	else if(cvar == g_pD3DXShift)
		return CVar_D3DXShift((CVar*)cvar,parms.IntTok(1));
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
	g_pConsole->RegisterCommand("tga_shot",CMD_TGASHOT,this);
	g_pConsole->RegisterCommand("pcx_shot",CMD_PCXSHOT,this);

	g_pFullbright= new CVar("r_fullbright","0",CVAR_INT,CVAR_ARCHIVE);
	g_pFov		 = new CVar("r_fov","90", CVAR_INT,CVAR_ARCHIVE);
	g_pVidSynch  = new CVar("r_vidsynch","0",CVAR_INT, CVAR_ARCHIVE);
	g_pMultiTexture = new CVar("r_multitexture","1", CVAR_INT,CVAR_ARCHIVE);
	g_p32BitTextures = new CVar("r_32bittextures","1", CVAR_BOOL,CVAR_ARCHIVE);
	g_pBeamTolerance = new CVar("r_beamtolerance","25", CVAR_FLOAT,CVAR_ARCHIVE);
	g_pD3DXShift = new CVar("r_d3dx_text_shift", "0", CVAR_INT, CVAR_ARCHIVE);

	g_pConsole->RegisterCVar(g_pFullbright,this);
	g_pConsole->RegisterCVar(g_pFov,this);
	g_pConsole->RegisterCVar(g_pMultiTexture,this);
	g_pConsole->RegisterCVar(g_pVidSynch,this);
	g_pConsole->RegisterCVar(g_p32BitTextures,this);
	g_pConsole->RegisterCVar(g_pBeamTolerance,this);
	g_pConsole->RegisterCVar(g_pD3DXShift,this);
}

/*
==========================================
Called from console Destructor
==========================================
*/
void CRConsole::DestroyConObjects()
{
	delete g_pFullbright;
	delete g_pFov;
	delete g_pVidSynch;
	delete g_pMultiTexture;
	delete g_p32BitTextures;
	delete g_pBeamTolerance;
	delete g_pD3DXShift;

	g_pFullbright = 0;
	g_pFov = 0;
	g_pVidSynch =0;
	g_pMultiTexture = 0;
	g_p32BitTextures = 0;
	g_pBeamTolerance = 0;
	g_pD3DXShift = 0;
}

