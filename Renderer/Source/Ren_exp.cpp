#include "Standard.h"
#include "Ren_exp.h"
#include "Ren_main.h"
#include "Ren_cache.h"
#include "Ren_beam.h"
#include "Hud_main.h"
#include "Tex_main.h"
#include "Mdl_main.h"
#include "Con_main.h"

#include "gl_rast.h"

//======================================================================================
//======================================================================================
//Global Vars
RenderInfo_t  g_rInfo;			//Shared Rendering Info
world_t		* world=0;			//The World
CRConsole   * g_prCons=0;

//======================================================================================
//======================================================================================

/*
=======================================
Constructor 
Creates all the objects and registers CVars
the Renderer itself wont be initialized until the 
configs have been excuted to update the cvars with the
saved rendering info
=======================================
*/
CRenExp::CRenExp() : m_cFull("r_full","0", CVar::CVAR_INT,CVar::CVAR_ARCHIVE),
					 m_cBpp("r_bpp", "16",CVar::CVAR_INT,CVar::CVAR_ARCHIVE),
					 m_cRes("r_res", "640 480",CVar::CVAR_STRING,CVar::CVAR_ARCHIVE)
{
	//Create different subsystems

	//Start the console first thing
	g_prCons= new CRConsole();
	g_pTex  = new CTextureManager();
	g_prHud = new CRHud();
	g_pRast = new COpenGLRast();

	g_pConsole->RegisterCVar(&m_cFull,this);
	g_pConsole->RegisterCVar(&m_cBpp,this);
	g_pConsole->RegisterCVar(&m_cRes,this);
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

	if (g_pRast)
		delete (g_pRast);
	g_pRast = 0;

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
	g_rInfo.bpp = m_cBpp.ival;
	if(m_cFull.ival)
		g_rInfo.rflags |= RFLAG_FULLSCREEN;
	else
		g_rInfo.rflags &= ~RFLAG_FULLSCREEN;


	//parse width and height out of string
	char *c = strchr(m_cRes.string, ' ');
	if(c)
	{
		char tmp[8];
		
		//Read Width
		int i = c - m_cRes.string;
		strncpy(tmp,m_cRes.string,i);
		sscanf(tmp,"%d",&g_rInfo.width);

		//Read Height
		c++;
		strcpy(tmp,m_cRes.string+i+1);
		sscanf(tmp,"%d",&g_rInfo.height);
	}

	//Intialize the Console now
	g_prCons->Init(true, true);
	g_prCons->UpdateRes();

	ConPrint("\n***** Renderer Intialization *****\n\n");

	//Start up opengl
	if(!g_pRast->Init())
	{
		ConPrint("CRenExp::InitRenderer:Failed to Init Rasterizer");
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
	ConPrint("Shutting donw renderer\n");

	//Destroy Subsystems
	cache_destroy();
	beam_shutdown();

	g_pTex->Shutdown();
	g_pRast->Shutdown();
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
void CRenExp::DrawFrame(vector_t *origin, vector_t *angles, vector_t *blend)
{
	//draw fullscreen console
	if (world == NULL)
	{
		r_drawcons();
		return;		
	}

	//draw a frame from the current viewpoint
	r_drawframe(origin,angles,blend);

// make sure all was well
#ifdef _DEBUG
	g_pRast->ReportErrors();
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
		g_pRast->SetWindowCoords(x,y);
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
		g_pRast->Resize();
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
	// shut the thing down
	g_pTex->Shutdown();

	g_pRast->UpdateDisplaySettings(width,height,bpp,fullscreen);

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


	// shut the thing down
	g_pRast->Shutdown();
	g_pRast->Init();

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

//======================================================================================
//======================================================================================

/*
==========================================
CVar Handlers
==========================================
*/
bool CRenExp::HandleCVar(const CVarBase *cvar,const CParms &parms)
{
	if(cvar == &m_cFull)
		return CVar_FullScreen((CVar*)cvar, parms);
	else if(cvar == &m_cRes)
		return CVar_Res((CVar*)cvar, parms);
	else if(cvar == &m_cBpp)
		return CVar_Bpp((CVar*)cvar, parms);
	return false;
}

/*
==========================================
CVar validation functions
==========================================
*/
bool CRenExp::CVar_FullScreen(const CVar * var, const CParms &parms)
{
	int argc = parms.NumTokens();
	if(argc ==1)
	{	if(var->ival ==0)
			ConPrint("In windowed mode\n");
		else
			ConPrint("In fullscreen mode\n");
	}
	else if(argc > 1)
	{
		int temp= parms.IntTok(1);
//		if((argv[1]) && (sscanf(argv[1],"%d",&temp)))
//		{
			if(temp <= 0)
			{
				if(!(g_rInfo.rflags & RFLAG_FULLSCREEN))
				{	ConPrint("Already in windowed mode\n");
					return false;
				}
				ConPrint("Switching to windowed mode\n");
				
				if(g_rInfo.ready)
					ChangeDispSettings(g_rInfo.width, g_rInfo.height, g_rInfo.bpp, false);
			}
			else if( temp > 0)
			{
				if(g_rInfo.rflags & RFLAG_FULLSCREEN)
				{	ConPrint("Already in fullscreen mode\n");
					return false;
				}
				ConPrint("Switching to fullscreen mode\n");
				
				if(g_rInfo.ready)
					ChangeDispSettings(g_rInfo.width, g_rInfo.height, g_rInfo.bpp, true);
			}
			return true;
//		}
	}
	return false;
}

/*
==========================================
Handle Resolution changes
==========================================
*/
bool CRenExp::CVar_Res(const CVar * var, const CParms &parms)
{
	int argc = parms.NumTokens();
	if(argc ==1)
		ConPrint("Running in %s resolution\n", var->string);
	else if(argc >2)
	{
		uint x=0, y=0;

		x = parms.IntTok(1);
		y = parms.IntTok(2);

		if(!x || !y)
		{
			ComPrintf("CVar_Res::Bad entries\n");
			return false;
		}

/*		if(!sscanf(argv[1],"%d",&x))
		{
			ConPrint("CVar_Res::Bad entry for Horizontal Resolution, Defaulting\n");
			x = g_rInfo.width;
		}
		if(!sscanf(argv[2],"%d",&y))
		{
			ConPrint("CVar_Res::Bad entry for Vertical Resolution, Defaulting\n");
			y = g_rInfo.height;
		}
*/
		// no go if we're not changing
		if ((g_rInfo.width == x) && (g_rInfo.height == y))
		{
			ConPrint("Already running in given resolution\n");
			return false;
		}

		ConPrint("Switching to %d x %d x %d bpp\n", x,y, g_rInfo.bpp );

		if(g_rInfo.ready)
			ChangeDispSettings(x, y, g_rInfo.bpp,(g_rInfo.rflags & RFLAG_FULLSCREEN));
		return true;
	}
	return false;
}

/*
==========================================
Handle bpp changes
==========================================
*/
bool CRenExp::CVar_Bpp(const CVar * var, const CParms &parms)
{
	int argc = parms.NumTokens();
	if(argc==1)
	{	ConPrint("Running in %d bpp\n", var->ival);
	}
	else if(argc > 1)
	{
		uint bpp= parms.IntTok(1);
		//if(!sscanf(argv[1],"%d",&bpp))
		//	bpp  = g_rInfo.bpp;

		// no go if we're not changing
		if (bpp == g_rInfo.bpp)
		{
			ConPrint("Already at given bpp\n");
			return false;
		}

		if(bpp < 0)
		{
			ConPrint("Bad Bpp\n");
			return false;
		}

		ConPrint("Switching to %d by %d at %d bpp\n", g_rInfo.width, g_rInfo.height, bpp);

		if(g_rInfo.ready)
			ChangeDispSettings(g_rInfo.width,g_rInfo.height,bpp, 
							(g_rInfo.rflags & RFLAG_FULLSCREEN));
		return true;
	}
	return false;
}