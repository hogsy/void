#include "Con_main.h"
#include "Tex_image.h"

/*
==========================================
Hack
Extension of the Renderer Console to manage
initialization/registration of global commands/objects
shouldnt be any global crap :\
==========================================
*/

CVar *	g_pFullbright=0;
CVar *	g_pFov=0;
CVar *	g_pMultiTexture=0;
CVar *	g_pVidSynch=0;
CVar *	g_p32BitTextures=0;
CVar *	g_pConAlpha=0;


//======================================================================================
//Command Handling routines
//======================================================================================

/*
======================================
Take a screen shot, write it to disk
======================================
*/
void ScreenShot(char *name, EImageFileFormat type)
{
	
	char	checkname[260];

	//find a file name to save it to 
	sprintf(checkname, "%s\\%s\\", GetCurPath(), "Shots");
	FileUtil::ConfirmDir(checkname);

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
			ConPrint("too many screen shots - try deleteing or moving some\n"); 
			return;
		}
	}
	else
	{
		sprintf(checkname, "%s\\%s\\%s", GetCurPath(), "Shots", name);
		if(type == FORMAT_PCX)
			Util::SetDefaultExtension(checkname, ".pcx");
		else
			Util::SetDefaultExtension(checkname, ".tga");
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
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	CImageWriter imageWriter(width,height,data);
	imageWriter.Write(checkname,type);

	g_pHunkManager->HunkFree(data);
}

/*
==========================================
Take a PCX screenshot
==========================================
*/
void CFunc_PCXShot(int argc, char** args)
{
	if (argc > 1)
	{
		if (strlen(args[1]) > 20)
		{
			ConPrint("filename too long!\n");
			ScreenShot(0,FORMAT_PCX);
		}
		else
			ScreenShot(args[1], FORMAT_PCX);
	}
	else
		ScreenShot(NULL,FORMAT_PCX);
}

/*
==========================================
Take a TGA screenshot
==========================================
*/
void CFunc_TGAShot(int argc, char** args)
{
	if (argc > 1)
	{
		if (strlen(args[1]) > 20)
		{
			ConPrint("filename too long!\n");
			ScreenShot(0, FORMAT_TGA);
		}
		else
			ScreenShot(args[1],FORMAT_TGA);
	}
	else
		ScreenShot(0,FORMAT_TGA);
}

//======================================================================================
//CVar Handling funcs
//======================================================================================

/*
=======================================
switch fullbright (light) rendering
=======================================
*/
bool CVar_FullBright(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
		ConPrint("r_fullbright = %d\n", var->ival);
	else
	{
		int temp=0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if (temp)
				g_rInfo.rflags |= RFLAG_FULLBRIGHT;
			else
				g_rInfo.rflags &= ~RFLAG_FULLBRIGHT;
		}
	}
	return true;
}

/*
=======================================
toggle multitexture/multipass rendering
=======================================
*/
bool CVar_MultiTexture(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
	{
		if (!(g_rInfo.rflags & RFLAG_MULTITEXTURE))
			ConPrint("Your video card does not support ARB multitexturing.\n");
		ConPrint("multitexturing is %d\n", var->ival);
		return false;
	}
	return true;
}


/*
=======================================
switch fullbright (light) rendering
=======================================
*/
bool CVar_Fov(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
		ConPrint("r_fov = %d\n", var->ival);
	else
	{
		int temp=0;

		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if (temp && temp>=10 && temp<= 170)
			{
				g_rInfo.fov = temp * PI/180;
				float x = (float) tan(g_rInfo.fov * 0.5f);
				float z = x * 0.75f;						// always render in a 3:4 aspect ratio

				/* set viewing projection */
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glFrustum(-x, x, -z, z, 1, 10000);
				return true;
			}
		}
		return false;
	}
	return true;
}

/*
=======================================
toggle vid synch
=======================================
*/
bool CVar_VidSynch(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
		ConPrint("r_vidsynch = %d\n", var->ival);
	else
	{
		int temp=0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if (g_rInfo.rflags & RFLAG_SWAP_CONTROL)
			{
				if (temp)
					wglSwapIntervalEXT(1);
				else
					wglSwapIntervalEXT(0);
			}
		}
		else
			return false;
	}
	return true;
}


/*
=======================================
16 / 32 bit textures
=======================================
*/
bool CVar_32BitTextures(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
		return true;

	else
	{
		int temp=0;
		if(!argv[1] || !sscanf(argv[1],"%d",&temp))
			return false;
	}

	ConPrint("Change will take effect on next level load.\n");
	return true;
}


/*
=======================================
set how much the console will fade in
=======================================
*/
bool CVar_ConAlpha(const CVar * var, int argc, char** argv)
{
	if (argc>1)
	{
		int temp=0;
		if(!argv[1] || !sscanf(argv[1],"%d",&temp))
			return false;

		if (temp<100 || temp>255)
			return false;
	}

	return true;
}



//======================================================================================
//======================================================================================

#define CMD_TGASHOT	0
#define CMD_PCXSHOT	1

/*
==========================================
Handle Global Commands
==========================================
*/
void CRConsole::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case CMD_TGASHOT:
		CFunc_TGAShot(numArgs,szArgs);
		break;
	case CMD_PCXSHOT:
		CFunc_PCXShot(numArgs,szArgs);
		break;
	}
}

/*
==========================================
Handle Cvars
==========================================
*/
bool CRConsole::HandleCVar(const CVarBase *cvar,int numArgs, char ** szArgs)
{
	if(cvar == g_pFullbright)
		return CVar_FullBright((CVar*)cvar,numArgs,szArgs);
	else if(cvar == g_pFov)
		return CVar_Fov((CVar*)cvar,numArgs,szArgs);
	else if(cvar == g_pMultiTexture)
		return CVar_MultiTexture((CVar*)cvar,numArgs,szArgs);
	else if(cvar == g_pVidSynch)
		return CVar_VidSynch((CVar*)cvar,numArgs,szArgs);
	else if(cvar == g_p32BitTextures)
		return CVar_32BitTextures((CVar*)cvar,numArgs,szArgs);
	else if(cvar == g_pConAlpha)
		return CVar_ConAlpha((CVar*)cvar,numArgs,szArgs);
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

	g_pFullbright= new CVar("r_fullbright","0",CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
	g_pFov		 = new CVar("r_fov","90", CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
	g_pVidSynch  = new CVar("r_vidsynch","0",CVar::CVAR_INT, CVar::CVAR_ARCHIVE);
	g_pMultiTexture = new CVar("r_multitexture","1", CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
	g_p32BitTextures = new CVar("r_32bittextures","1", CVar::CVAR_BOOL,CVar::CVAR_ARCHIVE);
	g_pConAlpha = new CVar("r_conalpha","200", CVar::CVAR_INT,CVar::CVAR_ARCHIVE);


	g_pConsole->RegisterCVar(g_pFullbright,this);
	g_pConsole->RegisterCVar(g_pFov,this);
	g_pConsole->RegisterCVar(g_pMultiTexture,this);
	g_pConsole->RegisterCVar(g_pVidSynch,this);
	g_pConsole->RegisterCVar(g_p32BitTextures,this);
	g_pConsole->RegisterCVar(g_pConAlpha,this);
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
	delete g_pConAlpha;

	g_pFullbright = 0;
	g_pFov = 0;
	g_pVidSynch =0;
	g_pMultiTexture = 0;
	g_p32BitTextures = 0;
	g_pConAlpha = 0;
}


/*
FIX ME !!
why does this leave a leak ?
the default runtime delete is called in the CVar destructor instead of ours
WHY ??

class CBlah
{
public:
	CBlah() : g_pFullbright("r_fullbright","0",CVar::CVAR_INT,CVar::CVAR_ARCHIVE) {}
	~CBlah() {}

	CVar	g_pFullbright;
};
CBlah bha;
*/

