#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

//Forward declarations
class  CSoundManager;
class  CMusic;
class  CWorld;
class  CNetClient;
class  CGameClient;
class  CClientExports;

struct I_Renderer;
struct I_HudRenderer;
struct I_ClientRenderer;

/*
=====================================
The Main Client class
=====================================
*/
class CClient :	public I_ConHandler
{
public:
	CClient(I_Renderer * pRenderer,	
			CSoundManager * pSound,	
			CMusic	* pMusic);
	~CClient();

	void RunFrame();
	void SetInputState(bool on);

	//Console Interface
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	
private:
	
	friend class CClientExports;

	//Hacks 
//	void SetClientState(int state);
	void HandleNetEvent(int event);
	void SetNetworkRate(int rate);

	CWorld * LoadWorld(const char *worldname);
	void UnloadWorld();

	void WriteUpdate();
	void ShowNetStats();

	//==================================================
	//Client CVars
	CVar		m_cvPort;
	CVar		m_cvNetStats;

//	int			m_clientState;
	bool	m_bInGame;
	float		m_fFrameTime;
	CWorld *	m_pWorld;
	
	CGameClient		  *	m_pClState;
	CClientExports	  * m_pExports;

	//Subsystems
	I_Renderer		  * m_pRender;
	I_ClientRenderer  * m_pClRen;
	I_HudRenderer	  * m_pHud;

	CSoundManager	  * m_pSound;
	CMusic		      * m_pMusic;
	CNetClient		  * m_pNetCl;
};

#endif