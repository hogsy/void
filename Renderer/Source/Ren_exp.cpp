#include "Standard.h"

#include "Ren_exp.h"
#include "Ren_main.h"
#include "Ren_cache.h"
#include "Ren_beam.h"

#include "Gl_main.h"

#include "Hud_main.h"
#include "Tex_main.h"
#include "Mdl_main.h"


//Static CVars
CVar *	CRenExp::m_cFull=0;		//Fullscreen
CVar *	CRenExp::m_cRes=0;		//Resolution
CVar *  CRenExp::m_cBpp=0;		//Bpp
CVar *	CRenExp::m_cWndX=0;		//Windowed X pos
CVar *  CRenExp::m_cWndY=0;		//Windowed Y pos
CVar *  CRenExp::m_cGLExt=0;	//Store GL Exts

//Global Vars
RenderInfo_t* rInfo=0;			//Shared Rendering Info
world_t		* world=0;			//The World

float		* g_pCurTime=0;		//Current Timer
float		* g_pFrameTime=0;	//Frame Time
char		* g_szGamePath=0;	//Game Path

CRenExp		* g_pRenExp=0;

/*
=======================================
Constructor 
=======================================
*/
CRenExp::CRenExp()
{	m_pImport = 0;
}

/*
==========================================
Destructor
==========================================
*/
CRenExp::~CRenExp()
{	m_pImport = 0;
}


/*
==========================================
Init
Creates all the objects and registers CVars
the Renderer itself wont be initialized until the 
configs have been excuted to update the cvars with the
saved rendering info
==========================================
*/
bool CRenExp::PreInit(RenderInfo_t *rinfo, VoidExport_t ** vexp)
{
	rInfo = rinfo;
	rInfo->ready = false;

	m_pImport = *vexp;
	
	//Set Global Time pointers
	g_pCurTime = m_pImport->curtime; 
	g_pFrameTime = m_pImport->frametime;

	//Create different subsystems

	//Start the console first thing
	g_prCons= new CRConsole(m_pImport->vconsole);
	g_pTex  = new CTextureManager();
	g_prHud = new CRHud();
	
	g_pGL	= new CGLUtil();
	if(!g_pGL->IsDriverLoaded())
	{
		ConPrint("Unable to load opengl driver");
		Shutdown();
		return false;
	}

	g_szGamePath = new char[256];
	sprintf(g_szGamePath,"%s/%s",m_pImport->basedir,m_pImport->gamedir);

	g_prCons->RegCVar(&m_cFull,"r_full","0", CVar::CVAR_INT,CVar::CVAR_ARCHIVE,&CVar_FullScreen);
	g_prCons->RegCVar(&m_cBpp,"r_bpp",  "16",CVar::CVAR_INT,CVar::CVAR_ARCHIVE,&CVar_Bpp);
	g_prCons->RegCVar(&m_cWndX,"r_wndx","80",CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
	g_prCons->RegCVar(&m_cWndY,"r_wndy","40",CVar::CVAR_INT,CVar::CVAR_ARCHIVE);
	g_prCons->RegCVar(&m_cRes,"r_res",  "640 480",CVar::CVAR_STRING,CVar::CVAR_ARCHIVE,&CVar_Res);
	return true;
}


/*
==========================================
This is called once the CVars have been updated
in by the exe, so we directly start the renderer
in the correct mode
==========================================
*/

bool CRenExp::InitRenderer()
{
	//Setup initial state
	rInfo->bpp = (int)m_cBpp->value;
	if(m_cFull->value)
		rInfo->rflags |= RFLAG_FULLSCREEN;

	//parse width and height out of string
	int i=0;
	char *c = strchr(m_cRes->string, ' ');
	if(c)
	{
		char tmp[8];

		//Read Width
		i = c - m_cRes->string;
		strncpy(tmp,m_cRes->string,i);
		sscanf(tmp,"%d",&rInfo->width);

		//Read Height
		c++;
		strcpy(tmp,m_cRes->string+i+1);
		sscanf(tmp,"%d",&rInfo->height);

		rInfo->width;
		rInfo->height;
	}

	//Intialize the Console now
	g_prCons->Init(true, true);
	ConPrint("\n***** Renderer Intialization *****\n\n");

	//Set all global pointers to null
	world	= NULL;

	//Start up opengl
	g_pGL->SetWindowCoords((int)m_cWndX->value,(int)m_cWndY->value);
	if(!g_pGL->Init())
	{
		ConPrint("CRenExp::InitRenderer:Failed to Init Opengl");
		Shutdown();
		return false;
	}

	//Print Extensions info
	ConPrint("\nGL_VENDOR: %s\n", glGetString(GL_VENDOR));
	ConPrint("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	ConPrint("GL_VERSION: %s\n", glGetString(GL_VERSION));

	const char *ext = (const char*)glGetString(GL_EXTENSIONS);
	int l = strlen(ext);
	char *ext2 = (char*)MALLOC(l);
	memcpy(ext2, ext, l);
	char *start = ext2;

	ConPrint("GL_EXTENSIONS:\n");
	for (i = 0; i < l; i++)
	{
		if (ext2[i] == ' ')
		{
			ext2[i] = NULL;
			ConPrint("%s\n", start);

			// check for extensions we want
			if (!strcmp(start, "GL_ARB_multitexture"))
				rInfo->rflags |= RFLAG_MULTITEXTURE;

			start = &ext2[i+1];
		}
	}
	free(ext2);

	//Start up the texture manager
	if(!g_pTex->Init(g_szGamePath))
	{
		ConPrint("failed to init texture base\n");
		return false;
	}

	//Start up the renderer
//	r_preinit(); 
	r_init();
//	cache_init();

	ConPrint("\n***** Renderer Initialization Successful *****\n\n");
	g_prCons->UpdateRes();
	return true;
}


/*
==========================================
Shutdown
==========================================
*/
bool CRenExp::Shutdown(void)
{
	//Update Win Pos
/*	RECT rect;
	if(GetWindowRect(rInfo->hWnd, &rect))
	{
		if(rect.left >= 40)
			m_cWndX->Set((float)rect.left);
		else
			m_cWndX->Set((float)40);

		if(rect.top >= 20)
			m_cWndY->Set((float)rect.top);
		else
			m_cWndY->Set((float)20);
	}
*/

//Test, WHY DOES THIS LEAVE A MEM LEAK
/*
	free(m_cWndX->default_string);
	m_cWndX->default_string = (char *)MALLOC(8);
	strcpy(m_cWndX->default_string,"heh");
*/

	//Destroy Subsystems
	cache_destroy();
	beam_shutdown();

	g_pTex->Shutdown();
	if(g_pTex)
		delete g_pTex;

	g_pGL->Shutdown();
	if(g_pGL)
		delete g_pGL;

	if(g_prHud)
		delete g_prHud;

	g_prCons->Shutdown();
	if(g_prCons)
		delete g_prCons;
	
	delete [] g_szGamePath;
	g_szGamePath = 0;

	rInfo = 0;
	return true;
}

/*
==========================================
Returns the Console Interface
==========================================
*/
I_RConsole * CRenExp::GetConsole()
{	return g_prCons;
}

/*
==========================================
Return the Hud interface
==========================================
*/
I_RHud * CRenExp::GetHud()
{	return g_prHud;
}


/*
==========================================
DrawFrame
==========================================
*/
void CRenExp::DrawFrame(vector_t *origin, vector_t *angles)
{
// FIXME - don't *always* do this
//	_wglMakeCurrent(rInfo->hDC, rInfo->hRC);

	//draw fullscreen console
	if (world == NULL)
	{
		r_drawcons();
		return;		
	}

	//draw a frame from the current viewpoint
	r_drawframe(origin,angles);

// make sure all was well
#ifdef _DEBUG
	int err = glGetError();
	if (err != GL_NO_ERROR)
	{
		ConPrint("CRenExp::DrawFrame:GL ERROR: %s", err);
	}
#endif

}


/*
==========================================
On Move Window
==========================================
*/
void CRenExp::MoveWindow(int x, int y)
{
	//Update new xy-coords if in windowed mode
	if(rInfo && !(rInfo->rflags & RFLAG_FULLSCREEN))
	{	g_pGL->SetWindowCoords(x,y);
	}
}

/*
==========================================
On Resize Window
==========================================
*/
void CRenExp::Resize()
{
	if (rInfo && rInfo->ready)
	{
		g_pGL->Resize();
		g_prCons->UpdateRes();
	}
}


/*
==========================================
Load a World
==========================================
*/
bool CRenExp::LoadWorld(world_t *level, int reload)
{
	if(!world)
	{
		_wglMakeCurrent(rInfo->hDC, rInfo->hRC);

		if(!g_pTex->UnloadWorldTextures())
		{
			ConPrint("CRenExp::LoadWorld::Error Unloading textures\n");
			return false;
		}
		
		model_destroy_map();
		world = level;
		
		if(!g_pTex->LoadWorldTextures(world))
		{
			ConPrint("CRenExp::LoadWorld::Error Reloading textures\n");
			return false;
		}
		model_load_map();
		return true;
	}
	ConPrint("CRenExp::LoadWorld::Error Bad World Pointer\n");
	return false;
}


/*
======================================
Unload World
======================================
*/
bool CRenExp::UnloadWorld()
{
	// get rid of all the old textures, load the new ones
	if(world)
	{
		_wglMakeCurrent(rInfo->hDC, rInfo->hRC);
		
		g_pTex->UnloadWorldTextures();
		model_destroy_map();

		world = 0;
		return true;
	}
	ConPrint("CRenExp::UnloadWorld::Error No World to Unload\n");
	return false;
}

/*
==========================================
change res or to/from fullscreen
==========================================
*/

void CRenExp::ChangeDispSettings(unsigned int width, unsigned int height, unsigned int bpp, 
								 unsigned int fullscreen)
{
	//Update GameDir
	sprintf(g_szGamePath,"%s/%s",m_pImport->basedir,m_pImport->gamedir);

	_wglMakeCurrent(rInfo->hDC, rInfo->hRC);
	
	// shut the thing down
	g_pTex->Shutdown();

	g_pGL->UpdateDisplaySettings(width,height,bpp,fullscreen);

	if(!g_pTex->Init(g_szGamePath))
	{
		ConPrint("failed to init texture base\n");
		return;
	}

	// reload our textures and models
	if(world)
	if(!g_pTex->LoadWorldTextures(world))
	{
		FError("ChangeDispSettings::Error Reloading textures\n");
	}

	model_load_map();

	// make sure our console is up to date
	g_prCons->UpdateRes();

	r_init();
}


/*
==========================================
Restart the Renderer
==========================================
*/
bool CRenExp::Restart(void)
{
	//Update GameDir
	sprintf(g_szGamePath,"%s/%s",m_pImport->basedir,m_pImport->gamedir);

	g_pTex->Shutdown();

	_wglMakeCurrent(rInfo->hDC, rInfo->hRC);
	
	// shut the thing down
	g_pGL->Shutdown();
	g_pGL->Init();

	//Start up the texture manager
	if(!g_pTex->Init(g_szGamePath))
	{
		ConPrint("failed to init texture base\n");
		return false;
	}
	
	// reload our textures
	if(!g_pTex->LoadWorldTextures(world))
	{
		ConPrint("Restart::Error Reloading textures\n");
	}

	//reload our models
	model_load_map();

	//make sure our console is up to date
	g_prCons->UpdateRes();

	r_init();

	return true;
}



/*
==========================================
CVar validation functions
==========================================
*/

bool CRenExp::CVar_FullScreen(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
	{	if(var->value==0)
			ConPrint("In windowed mode\n");
		else
			ConPrint("In fullscreen mode\n");
	}
	else if(argc >= 2)
	{
		int temp=0;
		if((argv[1]) && (sscanf(argv[1],"%d",&temp)))
		{
			if(temp <= 0)
			{
				if(!(rInfo->rflags & RFLAG_FULLSCREEN))
				{	ConPrint("Already in windowed mode\n");
					return false;
				}
				ConPrint("Switching to windowed mode\n");
				
				if(rInfo->ready)
					g_pRenExp->ChangeDispSettings(rInfo->width, rInfo->height, rInfo->bpp, false);
			}
			else if( temp > 0)
			{
				if(rInfo->rflags & RFLAG_FULLSCREEN)
				{	ConPrint("Already in fullscreen mode\n");
					return false;
				}
				ConPrint("Switching to fullscreen mode\n");
				
				if(rInfo->ready)
					g_pRenExp->ChangeDispSettings(rInfo->width, rInfo->height, rInfo->bpp, true);
			}
			return true;
		}
	}
	return false;
}

/*
==========================================
Handle Resolution changes
==========================================
*/
bool CRenExp::CVar_Res(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
	{	ConPrint("Running in %s resolution\n", var->string);
	}
	else if(argc >=3)
	{
		int x=0, y=0;

		if(!sscanf(argv[1],"%d",&x))
		{
			ConPrint("CVar_Res::Bad entry for Horizontal Resolution, Defaulting\n");
			x = rInfo->width;
		}
		if(!sscanf(argv[2],"%d",&y))
		{
			ConPrint("CVar_Res::Bad entry for Vertical Resolution, Defaulting\n");
			y = rInfo->height;
		}

		// no go if we're not changing
		if ((rInfo->width == x) && (rInfo->height == y))
		{
			ConPrint("Already running in given resolution\n");
			return false;
		}

		ConPrint("Switching to %d x %d x %d bpp\n", x,y, rInfo->bpp );

		if(rInfo->ready)
			g_pRenExp->ChangeDispSettings(x, y, rInfo->bpp, (rInfo->rflags & RFLAG_FULLSCREEN) ? true : false);
		return true;
	}
	return false;
}

/*
==========================================
Handle bpp changes
==========================================
*/
bool CRenExp::CVar_Bpp(const CVar * var, int argc, char** argv)
{
	if(argc<=1)
	{	ConPrint("Running in %d bpp\n", (int)var->value);
	}
	else if(argc >= 2)
	{
		int bpp=0;
		if(!sscanf(argv[1],"%d",&bpp))
			bpp  = rInfo->bpp;

		// no go if we're not changing
		if (bpp == rInfo->bpp)
		{
			ConPrint("Bad Entry or already at given bpp\n");
			return false;
		}

		ConPrint("Switching to %d by %d at %d bpp\n", rInfo->width,rInfo->height,bpp );

		if(rInfo->ready)
		g_pRenExp->ChangeDispSettings(rInfo->width,rInfo->height,bpp, 
						(rInfo->rflags & RFLAG_FULLSCREEN) ? true : false);
		return true;
	}
	return false;
}



