#include "Standard.h"
#include "Com_parms.h"
#include "Ren_exp.h"
#include "Ren_main.h"
#include "Ren_cache.h"
#include "Ren_beam.h"
#include "Tex_main.h"
#include "Tex_image.h"
#include "Con_main.h"
#include "Hud_main.h"
#include "Client.h"
#include "ShaderManager.h"

#include "gl_rast.h"
#include "Rast_none.h"
#include "Rast_d3dx.h"

//======================================================================================
//======================================================================================

//Global Vars
RenderInfo_t		g_rInfo;			//Shared Rendering Info
CWorld			*	world=0;			//The World

CClientRenderer *	g_pClient=0;
CRasterizer		*	g_pRast=0;

extern	CVar g_varFullbright;

/*
=======================================
Constructor 
Creates all the objects and registers CVars
the Renderer itself wont be initialized until the 
configs have been excuted to update the cvars with the
saved rendering info
=======================================
*/
CRenExp::CRenExp() : m_cFull("r_full","0", CVAR_BOOL,CVAR_ARCHIVE),
					 m_cBpp("r_bpp", "16",CVAR_INT,CVAR_ARCHIVE),
					 m_cRes("r_res", "640 480",CVAR_STRING,CVAR_ARCHIVE),
					 m_cRast("r_rast", "gl",CVAR_STRING, CVAR_ARCHIVE)
{
	//Create different subsystems

	//Start the console first thing
	m_pRConsole= new CRConsole();
	m_pHud     = new CRHud();

	g_pShaders = new CShaderManager();
	g_pTex     = new CTextureManager();
	g_pClient  = new CClientRenderer();
	
	I_Console::GetConsole()->RegisterCVar(&m_cRast, this);
	I_Console::GetConsole()->RegisterCVar(&m_cFull,this);
	I_Console::GetConsole()->RegisterCVar(&m_cBpp,this);
	I_Console::GetConsole()->RegisterCVar(&m_cRes,this);

	if (_stricmp(m_cRast.string, "gl")==0)
		g_pRast = new COpenGLRast();
	else if(_stricmp(m_cRast.string,"none")==0)
		g_pRast = new CRastNone();
	else
		g_pRast = new CRastD3DX();
}

/*
==========================================
Destructor
==========================================
*/
CRenExp::~CRenExp()
{
	CImageReader::GetReader().Shutdown();

	if (g_pShaders)
		delete g_pShaders;
	g_pShaders = 0;

	if(g_pTex)
		delete g_pTex;
	g_pTex = 0;

	if (g_pClient)
		delete g_pClient;
	g_pClient = 0;

	if( m_pHud)
		delete m_pHud;
	m_pHud = 0;

	if (g_pRast)
		delete g_pRast;
	g_pRast = 0;

	if(m_pRConsole)
		delete m_pRConsole;
	m_pRConsole = 0;
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
	if(!c)
		ComPrintf("CRenExp::InitRenderer::Unable to read x-y resolution from %s\n", m_cRes.string);
	else
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
	m_pRConsole->Init(true, true);
	m_pRConsole->UpdateRes();

	ComPrintf("\n***** Renderer Intialization *****\n\n");

	//Start up opengl
	if(!g_pRast->Init())
	{
		ComPrintf("CRenExp::InitRenderer:Failed to Init Rasterizer");
		Shutdown();
		return false;
	}

	g_pShaders->LoadBase();

	//Start up the texture manager
	if(!g_pTex->Init())
	{
		ComPrintf("CRenExp::InitRenderer:Failed to initialize Texture manager\n");
		Shutdown();
		return false;
	}

	//Update the res/bpp cvars
	char res[16];
	sprintf(res,"%d %d", g_rInfo.width, g_rInfo.height);
	m_cRes.ForceSet(res);
	m_cBpp.ForceSet((int)g_rInfo.bpp);

	//Start up the renderer
	r_init();

	ComPrintf("\n***** Renderer Initialization Successful *****\n\n");
	return true;
}


/*
==========================================
Shutdown
==========================================
*/
bool CRenExp::Shutdown(void)
{
	g_pRast->SetFocus();

	//Destroy Subsystems
	beam_shutdown();

	g_pClient->UnLoadTextures();
	g_pClient->UnLoadSkins();
	g_pShaders->UnLoadBase();
	g_pShaders->UnLoadWorld();
	g_pTex->Shutdown();
	g_pRast->Shutdown();
	m_pRConsole->Shutdown();
	return true;
}

/*
==========================================
Returns the Appropriate Interface
==========================================
*/
I_ConsoleRenderer * CRenExp::GetConsole()
{	return m_pRConsole;
}

I_ClientRenderer * CRenExp::GetClient()
{	return g_pClient;
}

I_HudRenderer	 * CRenExp::GetHud()
{	return m_pHud;
}



/*
==========================================
DrawFrame
==========================================
*/

void CRenExp::Draw(const CCamera * camera)
{
	g_pRast->SetFocus();
	g_pRast->ClearBuffers(/*VRAST_COLOR_BUFFER |*/ VRAST_DEPTH_BUFFER);

	// decide whether or not to use lights before doing anything
	if (!world)
		g_pRast->UseLights(false);
	else if (!g_varFullbright.bval  && world->light_size)
		g_pRast->UseLights(true);
	else
		g_pRast->UseLights(false);

	if(camera)
	{
		r_drawframe(camera);
		if (world)
			m_pHud->Printf(0, 160, 0, "tris: %d", g_pRast->GetNumTris());
		m_pHud->DrawHud();
	}
	m_pRConsole->Draw();
	
	g_pRast->FrameEnd();

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
	g_pRast->SetFocus();

	//Update new xy-coords if in windowed mode
	if(!(g_rInfo.rflags & RFLAG_FULLSCREEN))
		g_pRast->SetWindowCoords(x,y);
}

/*
==========================================
On Resize Window
==========================================
*/
void CRenExp::Resize()
{
	g_pRast->SetFocus();

	if (g_rInfo.ready)
	{
		g_pRast->Resize();
		m_pRConsole->UpdateRes();
	}
}


/*
==========================================
Load a World
==========================================
*/
bool CRenExp::LoadWorld(CWorld *level, int reload)
{
	g_pRast->SetFocus();

	if(!world && level)
	{
		g_pShaders->UnLoadWorld();
		if(!g_pTex->UnloadWorldTextures())
		{
			ComPrintf("CRenExp::LoadWorld::Error Unloading textures\n");
			return false;
		}
		
		world = level;

		g_pShaders->LoadWorld(world);
		if(!g_pTex->LoadWorldTextures(world))
		{
			ComPrintf("CRenExp::LoadWorld::Error Reloading textures\n");
			return false;
		}
		return true;
	}
	ComPrintf("CRenExp::LoadWorld::Error Bad World Pointer\n");
	return false;
}


/*
======================================
Unload World
======================================
*/
bool CRenExp::UnloadWorld()
{
	g_pRast->SetFocus();

	// get rid of all the old textures, load the new ones
	if(world)
	{
		g_pShaders->UnLoadWorld();
		g_pTex->UnloadWorldTextures();
		world = 0;
		return true;
	}
	ComPrintf("CRenExp::UnloadWorld::Error No World to Unload\n");
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
	g_pRast->SetFocus();

/*	// if we're not changing bpp, we can skip a lot of stuff
	if (bpp == g_rInfo.bpp)
	{
		g_pRast->UpdateDisplaySettings(width,height,bpp,fullscreen);
		m_pRConsole->UpdateRes();
		r_init();
		return;
	}
*/
	// shut the thing down
	g_pTex->Shutdown();
	g_pShaders->UnLoadBase();
	g_pShaders->UnLoadWorld();
	g_pClient->UnLoadSkins();
	g_pClient->UnLoadTextures();

	g_pRast->UpdateDisplaySettings(width,height,bpp,fullscreen);

	g_pShaders->LoadBase();

	if(!g_pTex->Init())
	{
		ComPrintf("failed to init texture base\n");
		return;
	}

	// reload our textures and models
	if(world)
	{
		g_pShaders->LoadWorld(world);
		if(!g_pTex->LoadWorldTextures(world))
		{
			FError("ChangeDispSettings::Error Reloading textures\n");
			return;
		}
	}

	g_pClient->LoadTextures();
	g_pClient->LoadSkins();

	// make sure our console is up to date
	m_pRConsole->UpdateRes();
	r_init();
}


/*
==========================================
Restart the Renderer
==========================================
*/
bool CRenExp::Restart(void)
{
	g_pRast->SetFocus();

	g_pTex->Shutdown();
	g_pShaders->UnLoadBase();
	g_pShaders->UnLoadWorld();
	g_pClient->UnLoadSkins();
	g_pClient->UnLoadTextures();

	// shut the thing down
	g_pRast->Shutdown();
	g_pRast->Init();

	//Start up the texture manager
	g_pShaders->LoadBase();
	if(!g_pTex->Init())
	{
		ComPrintf("failed to init texture base\n");
		return false;
	}

	// reload our textures
	g_pShaders->LoadWorld(world);
	if(!g_pTex->LoadWorldTextures(world))
	{
		ComPrintf("Restart::Error Reloading textures\n");
	}

	//reload our model skins
	g_pClient->LoadTextures();
	g_pClient->LoadSkins();

	//make sure our console is up to date
	m_pRConsole->UpdateRes();

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
bool CRenExp::HandleCVar(const CVarBase *cvar,const CStringVal &strVal)
{
	if(g_pRast)
		g_pRast->SetFocus();

	if(cvar == &m_cFull)
		return CVar_FullScreen(strVal);
	else if(cvar == &m_cRes)
		return CVar_Res(strVal);
	else if(cvar == &m_cBpp)
		return CVar_Bpp(strVal);
	else if(cvar == &m_cRast)
		return CVar_Rast(strVal);
	return false;
}

/*
==========================================
CVar validation functions
==========================================
*/
bool CRenExp::CVar_FullScreen(const CStringVal &strVal)
{
	int temp= strVal.IntVal();

	if(temp <= 0)
	{
		if(!(g_rInfo.rflags & RFLAG_FULLSCREEN))
		{	ComPrintf("Already in windowed mode\n");
			return false;
		}
		ComPrintf("Switching to windowed mode\n");
		
		if(g_rInfo.ready)
			ChangeDispSettings(g_rInfo.width, g_rInfo.height, g_rInfo.bpp, false);
	}
	else if( temp > 0)
	{
		if(g_rInfo.rflags & RFLAG_FULLSCREEN)
		{	ComPrintf("Already in fullscreen mode\n");
			return false;
		}
		ComPrintf("Switching to fullscreen mode\n");
		
		if(g_rInfo.ready)
			ChangeDispSettings(g_rInfo.width, g_rInfo.height, g_rInfo.bpp, true);
	}
	return true;
}

/*
==========================================
Handle Resolution changes
==========================================
*/
bool CRenExp::CVar_Res(const CStringVal &strVal)
{
	char resString[16];
	strcpy(resString, strVal.String());

	char *c = strchr(resString, ' ');
	if(!c)
	{
		ComPrintf("Unable to read resolution in \"%s\". Format should be \"x y\"\n", resString);
		return false;
	}
	
	char tmp[8];
	uint x=0, y=0;
	
	//Read Width
	int i = c - resString;
	strncpy(tmp,resString,i);
	sscanf(tmp,"%d",&x);

	//Read Height
	c++;
	strcpy(tmp,resString+i+1);
	sscanf(tmp,"%d",&y);

	if(x <= 0)
	{
		ComPrintf("CVar_Res::Bad entry for Horizontal Resolution\n");
		return false;
	}

	if(y <= 0)
	{
		ComPrintf("CVar_Res::Bad entry for Vertical Resolution\n");
		return false;
	}

	// no go if we're not changing
	if ((g_rInfo.width == x) && (g_rInfo.height == y))
	{
		ComPrintf("Already running in given resolution\n");
		return false;
	}

	ComPrintf("Switching to %d x %d x %d bpp\n", x,y, g_rInfo.bpp );

	if(g_rInfo.ready)
		ChangeDispSettings(x, y, g_rInfo.bpp,(g_rInfo.rflags & RFLAG_FULLSCREEN));
	return true;
}

/*
==========================================
Handle bpp changes
==========================================
*/
bool CRenExp::CVar_Bpp(const CStringVal &strVal)
{
	uint bpp= strVal.IntVal();

	// no go if we're not changing
	if (bpp == g_rInfo.bpp)
	{
		ComPrintf("Already at given bpp\n");
		return false;
	}

	if(bpp != 16 && bpp != 32)
	{
		ComPrintf("Bad Bpp\n");
		return false;
	}

	if(g_rInfo.ready)
	{
		ComPrintf("Switching to %d by %d at %d bpp\n", g_rInfo.width, g_rInfo.height, bpp);
		ChangeDispSettings(g_rInfo.width,g_rInfo.height,bpp, (g_rInfo.rflags & RFLAG_FULLSCREEN));
	}
	return true;
}


/*
==========================================
Handle rasterizer changes
==========================================
*/
bool CRenExp::CVar_Rast(const CStringVal &strVal)
{
	if((stricmp(strVal.String(),"d3dx")==0) ||
	   (stricmp(strVal.String(),"gl")==0) ||
	   (stricmp(strVal.String(),"none")==0))
	{
		ComPrintf("Rasterizer change will take effect next time you run void.\n");
		return true;
	}
	ComPrintf("Invalid driver specified\n");
	return false;
}