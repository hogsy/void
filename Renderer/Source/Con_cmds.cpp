#include "Con_main.h"
#include "Tex_image.h"

//======================================================================================
//======================================================================================

//CVar 	g_pDrawSils;

//CVar	g_pFullbright("r_fullbright","0",CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
/*CVar	g_pFov("r_fov","90", CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
CVar	g_pMultiTexture("r_multitexture","1", CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
CVar 	g_pVidSynch("r_vidsynch","0",CVar::CVAR_INT, CVar::CVAR_ARCHIVE);
*/

/*
======================================
take a screen shot
======================================
*/

void ScreenShot(char *name,EImageFormat type)
{
//taking this out for a bit
#if 0 

	char	checkname[260];

	//find a file name to save it to 
	sprintf(checkname, "%s\\%s\\", CFileSystem::GetCurrentPath(), "Shots");
	ConfirmDir(checkname);

	if (!name)
	{
		int i;
		FILE	*f;
		char	shotname[80]; 

		if(type== FORMAT_PCX)
			strcpy(shotname,"void00.pcx");
		else
			strcpy(shotname,"void00.tga");

		for (i=0 ; i<=99 ; i++) 
		{
			shotname[4] = i/10 + '0';
			shotname[5] = i%10 + '0';
			sprintf(checkname,"%s\\%s\\%s", CFileSystem::GetCurrentPath(), "Shots", shotname);
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
		sprintf(checkname, "%s\\%s\\%s", CFileSystem::GetCurrentPath(), "Shots", name);
		if(type == FORMAT_PCX)
			DefaultExtension(checkname, ".pcx");
		else
			DefaultExtension(checkname, ".tga");
	}

	//Got file name, Now actually take the shot and write it

	int  width  = g_rInfo.width;
	int  height = g_rInfo.height;
	byte * data = new byte[width * height * 4];

	if (data == NULL) 
	{
		Error("ScreenShot:No mem to capture screendata");
		return;
	}
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	CImageWriter imageWriter(width,height,data);
	imageWriter.Write(checkname,type);
	delete [] data;

#endif

}


//======================================================================================
//======================================================================================

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



/*
=======================================
switch fullbright (light) rendering
=======================================
*/
bool CVar_FullBright(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
		ConPrint("r_fullbright = %d\n", var->value);

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

#if 0

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
		ConPrint("multitexturing is %d\n", (int)var->value);
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
		ConPrint("r_fov = %d\n", var->value);
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
		ConPrint("r_vidsynch = %d\n", var->value);
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

#endif
bool CRConsole::HandleCVar(const CVar *cvar,int numArgs, char ** szArgs)
{
//	if(cvar == &g_pFullbright)
//		return CVar_FullBright(cvar,numArgs,szArgs);
/*	else if(cvar == &g_pFov)
		return CVar_Fov(cvar,numArgs,szArgs);
	else if(cvar == &g_pMultiTexture)
		return CVar_MultiTexture(cvar,numArgs,szArgs);
	else if(cvar == &g_pVidSynch)
		return CVar_VidSynch(cvar,numArgs,szArgs);
*/
	return false;
}

/*
==========================================
Register Cvars and Commands
==========================================
*/
void CRConsole::RegisterFuncs()
{
	g_pConsole->RegisterCVar(&m_conSpeed);

//	g_pConsole->RegisterCVar(&g_pFov,this);
//	g_pConsole->RegisterCVar(&g_pFullbright,this);
/*	g_pConsole->RegisterCVar(&g_pMultiTexture,this);
	g_pConsole->RegisterCVar(&g_pVidSynch,this);
*/

/*	g_pConsole->RegisterCVar(&g_pFov,"r_fov","90", CVar::CVAR_INT,CVar::CVAR_ARCHIVE,this);
	g_pConsole->RegisterCVar(&g_pFullbright,"r_fullbright","0",CVar::CVAR_INT,CVar::CVAR_ARCHIVE,this);
	g_pConsole->RegisterCVar(&g_pMultiTexture,"r_multitexture","1", CVar::CVAR_INT,CVar::CVAR_ARCHIVE,this);
	g_pConsole->RegisterCVar(&g_pVidSynch,"r_vidsynch","0",CVar::CVAR_INT, CVar::CVAR_ARCHIVE,this);
*/
//	g_pConsole->RegisterCFunc("screenshot", &CFunc_PCXShot);
//	g_pConsole->RegisterCFunc("tgashot", &CFunc_TGAShot);
}