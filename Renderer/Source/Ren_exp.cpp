#include "Standard.h"
#include "Ren_exp.h"
#include "Ren_main.h"
#include "Ren_cache.h"
#include "Ren_beam.h"
#include "Gl_main.h"
#include "Hud_main.h"
#include "Tex_main.h"
#include "Mdl_main.h"
#include "Con_main.h"

//======================================================================================
//======================================================================================
//Global Vars
RenderInfo_t  g_rInfo;			//Shared Rendering Info
world_t		* world=0;			//The World

CRenExp		* g_pRenExp=0;

//======================================================================================

//Static CVars
CVar *	CRenExp::m_cFull=0;		//Fullscreen
CVar *	CRenExp::m_cRes=0;		//Resolution
CVar *  CRenExp::m_cBpp=0;		//Bpp

//======================================================================================

bool CRenExp::HandleCVar(const CVar *cvar,int numArgs, char ** szArgs)
{
	if(cvar == m_cFull)
		return CVar_FullScreen(cvar,numArgs,szArgs);
	else if(cvar == m_cRes)
		return CVar_Res(cvar,numArgs,szArgs);
	else if(cvar == m_cBpp)
		return CVar_Bpp(cvar,numArgs,szArgs);
	return false;
}

/*
=======================================
Constructor 
Creates all the objects and registers CVars
the Renderer itself wont be initialized until the 
configs have been excuted to update the cvars with the
saved rendering info
=======================================
*/
CRenExp::CRenExp()
{
	//Create different subsystems

	//Start the console first thing
	g_prCons= new CRConsole();
	g_pTex  = new CTextureManager();
	g_prHud = new CRHud();
	
	g_pGL	= new CGLUtil();
	if(!g_pGL->IsDriverLoaded())
	{
		ConPrint("Unable to load opengl driver");
		Shutdown();
		return;
	}

	m_cFull= g_pConsole->RegisterCVar("r_full","0", CVar::CVAR_INT,CVar::CVAR_ARCHIVE,this);
	m_cBpp = g_pConsole->RegisterCVar("r_bpp", "16",CVar::CVAR_INT,CVar::CVAR_ARCHIVE,this);
	m_cRes = g_pConsole->RegisterCVar("r_res", "640 480",CVar::CVAR_STRING,CVar::CVAR_ARCHIVE,this);
}

/*
==========================================
Destructor
==========================================
*/
CRenExp::~CRenExp()
{
	if(g_pTex)
		delete g_pTex;
	g_pTex = 0;
	
	if(g_pGL)
		delete g_pGL;
	g_pGL = 0;
	
	if(g_prHud)
		delete g_prHud;
	g_prHud = 0;

	if(g_prCons)
		delete g_prCons;
	g_prCons = 0;

	g_pConsole = 0;
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
	//Set all global pointers to null
	world	= NULL;

	//Setup initial state
	g_rInfo.bpp = (int)m_cBpp->value;
	if(m_cFull->value)
		g_rInfo.rflags |= RFLAG_FULLSCREEN;

	//parse width and height out of string
	char *c = strchr(m_cRes->string, ' ');
	if(c)
	{
		char tmp[8];
		
		//Read Width
		int i = c - m_cRes->string;
		strncpy(tmp,m_cRes->string,i);
		sscanf(tmp,"%d",&g_rInfo.width);

		//Read Height
		c++;
		strcpy(tmp,m_cRes->string+i+1);
		sscanf(tmp,"%d",&g_rInfo.height);
	}

	//Intialize the Console now
	g_prCons->Init(true, true);
	g_prCons->UpdateRes();

	ConPrint("\n***** Renderer Intialization *****\n\n");

	//Start up opengl
	g_pGL->SetInitializePos();
	if(!g_pGL->Init())
	{
		ConPrint("CRenExp::InitRenderer:Failed to Init Opengl");
		Shutdown();
		return false;
	}

	//Start up the texture manager
	if(!g_pTex->Init())
	{
		ConPrint("CRenExp::InitRenderer:Failed to initialize Texture manager\n");
		Shutdown();
		return false;
	}

	//Start up the renderer
	r_init();

	ConPrint("\n***** Renderer Initialization Successful *****\n\n");
	return true;
}


/*
==========================================
Shutdown
==========================================
*/
bool CRenExp::Shutdown(void)
{
	//Destroy Subsystems
	cache_destroy();
	beam_shutdown();

	g_pTex->Shutdown();
	g_pGL->Shutdown();
	g_prCons->Shutdown();
	return true;
}

/*
==========================================
Returns the Console Interface
==========================================
*/
I_ConsoleRenderer * CRenExp::GetConsole()
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
	if(!(g_rInfo.rflags & RFLAG_FULLSCREEN))
	{	
		g_pGL->SetWindowCoords(x,y);
	}
}

/*
==========================================
On Resize Window
==========================================
*/
void CRenExp::Resize()
{
	if (g_rInfo.ready)
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
	if(!world && level)
	{
		_wglMakeCurrent(g_rInfo.hDC, g_rInfo.hRC);

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
		_wglMakeCurrent(g_rInfo.hDC, g_rInfo.hRC);
		
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
void CRenExp::ChangeDispSettings(unsigned int width, 
								 unsigned int height, 
								 unsigned int bpp, 
								 bool fullscreen)
{
	_wglMakeCurrent(g_rInfo.hDC, g_rInfo.hRC);
	
	// shut the thing down
	g_pTex->Shutdown();

	g_pGL->UpdateDisplaySettings(width,height,bpp,fullscreen);

	if(!g_pTex->Init())
	{
		ConPrint("failed to init texture base\n");
		return;
	}

	// reload our textures and models
	if(world)
	if(!g_pTex->LoadWorldTextures(world))
	{
		FError("ChangeDispSettings::Error Reloading textures\n");
		return;
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
	g_pTex->Shutdown();

	_wglMakeCurrent(g_rInfo.hDC, g_rInfo.hRC);
	
	// shut the thing down
	g_pGL->Shutdown();
	g_pGL->Init();

	//Start up the texture manager
	if(!g_pTex->Init())
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
				if(!(g_rInfo.rflags & RFLAG_FULLSCREEN))
				{	ConPrint("Already in windowed mode\n");
					return false;
				}
				ConPrint("Switching to windowed mode\n");
				
				if(g_rInfo.ready)
					g_pRenExp->ChangeDispSettings(g_rInfo.width, g_rInfo.height, g_rInfo.bpp, false);
			}
			else if( temp > 0)
			{
				if(g_rInfo.rflags & RFLAG_FULLSCREEN)
				{	ConPrint("Already in fullscreen mode\n");
					return false;
				}
				ConPrint("Switching to fullscreen mode\n");
				
				if(g_rInfo.ready)
					g_pRenExp->ChangeDispSettings(g_rInfo.width, g_rInfo.height, g_rInfo.bpp, true);
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
		uint x=0, y=0;

		if(!sscanf(argv[1],"%d",&x))
		{
			ConPrint("CVar_Res::Bad entry for Horizontal Resolution, Defaulting\n");
			x = g_rInfo.width;
		}
		if(!sscanf(argv[2],"%d",&y))
		{
			ConPrint("CVar_Res::Bad entry for Vertical Resolution, Defaulting\n");
			y = g_rInfo.height;
		}

		// no go if we're not changing
		if ((g_rInfo.width == x) && (g_rInfo.height == y))
		{
			ConPrint("Already running in given resolution\n");
			return false;
		}

		ConPrint("Switching to %d x %d x %d bpp\n", x,y, g_rInfo.bpp );

		if(g_rInfo.ready)
			g_pRenExp->ChangeDispSettings(x, y, g_rInfo.bpp,(g_rInfo.rflags & RFLAG_FULLSCREEN));
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
		uint bpp=0;
		if(!sscanf(argv[1],"%d",&bpp))
			bpp  = g_rInfo.bpp;

		// no go if we're not changing
		if (bpp == g_rInfo.bpp)
		{
			ConPrint("Bad Entry or already at given bpp\n");
			return false;
		}

		ConPrint("Switching to %d by %d at %d bpp\n", g_rInfo.width, g_rInfo.height, bpp);

		if(g_rInfo.ready)
			g_pRenExp->ChangeDispSettings(g_rInfo.width,g_rInfo.height,bpp, 
							(g_rInfo.rflags & RFLAG_FULLSCREEN));
		return true;
	}
	return false;
}