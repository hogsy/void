#include "Standard.h"
#include "Tex_image.h"


CVar *	g_pFullbright;
CVar *	g_pDrawSils;
CVar *  g_pFov;
CVar *  g_pMultiTexture;

/*
======================================
take a screen shot
======================================
*/
void ScreenShot(char *name,int type)
{
	int     i; 
	char	shotname[80]; 
	char	checkname[260];
	FILE	*f;
	CImage  pic;

	if(!pic.SnapShot())
	{
		ConPrint("ScreenShot::Error reading screendata\n"); 
		return;
	}

	if(type == CImage::FORMAT_NONE)
		type = pic.format;

	// 
	// find a file name to save it to 
	// 
//	sprintf(checkname, "%s\\%s\\", rInfo->base_dir, "screenshots");
	sprintf(checkname, "%s\\%s\\", g_szGamePath, "Shots");
	ConfirmDir(checkname);


	if (!name)
	{
		if(type== CImage::FORMAT_PCX)
			strcpy(shotname,"void00.pcx");
		else
			strcpy(shotname,"void00.tga");

		for (i=0 ; i<=99 ; i++) 
		{
			shotname[4] = i/10 + '0';
			shotname[5] = i%10 + '0';

//			sprintf (checkname, "%s\\%s\\%s", rInfo->base_dir, "screenshots", shotname);
			sprintf(checkname, "%s\\%s\\%s", g_szGamePath, "Shots", name);

			f = fopen(checkname, "rb");
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
//		sprintf(checkname, "%s\\%s\\%s", rInfo->base_dir, "screenshots", name);
		sprintf(checkname, "%s\\%s\\%s", g_szGamePath, "Shots", name);
		if(type == CImage::FORMAT_PCX)
			DefaultExtension(checkname, ".pcx");
		else
			DefaultExtension(checkname, ".tga");
	}

	//save the pcx file 
	pic.Write(checkname,(CImage::EImageFormat)type);
}


/************************************************
take a screenshot
************************************************/
void cfunc_screenshot(int argc, char** args)
{
	if (argc > 1)
	{
		if (strlen(args[1]) > 20)
		{
			ConPrint("filename too long!\n");
			ScreenShot(NULL,0);
		}
		else
			ScreenShot(args[1],0);
	}
	else
		ScreenShot(NULL,CImage::FORMAT_PCX);
}


void cfunc_tgashot(int argc, char** args)
{
	if (argc > 1)
	{
		if (strlen(args[1]) > 20)
		{
			ConPrint("filename too long!\n");
			ScreenShot(NULL,0);
		}

		else
			ScreenShot(args[1],CImage::FORMAT_TGA);
	}
	else
		ScreenShot(0,CImage::FORMAT_TGA);
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
				rInfo->rflags |= RFLAG_FULLBRIGHT;
			else
				rInfo->rflags &= ~RFLAG_FULLBRIGHT;
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
		if (!(rInfo->rflags & RFLAG_MULTITEXTURE))
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
				rInfo->fov = temp * PI/180;
				float x = (float) tan(rInfo->fov * 0.5f);
				float z = x * 0.75f;						// always render in a 3:4 aspect ratio

				/* set viewing projection */
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glFrustum(-x, x, -z, z, 1, 10000);
			}
			else
				return false;
		}
		else
			return false;
	}

	return true;
}



/************************************************
register all commands/functions relevant to the renderer
************************************************/
void CRConsole::RegisterFuncs()
{
	
	RegCVar(&g_pFullbright,"r_fullbright","0",CVar::CVAR_INT,CVar::CVAR_ARCHIVE,&CVar_FullBright);
	RegCVar(&g_pConspeed,"r_conspeed","500",CVar::CVAR_INT,CVar::CVAR_ARCHIVE,0);
	RegCVar(&g_pFov, "r_fov", "90", CVar::CVAR_INT, CVar::CVAR_ARCHIVE, &CVar_Fov);
	RegCVar(&g_pMultiTexture, "r_multitexture", "1", CVar::CVAR_INT, CVar::CVAR_ARCHIVE, &CVar_MultiTexture);


	RegCFunc("screenshot", &cfunc_screenshot);
	RegCFunc("tgashot", &cfunc_tgashot);
}