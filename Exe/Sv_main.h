#ifndef VOID_NETWORK_SERVER
#define VOID_NETWORK_SERVER

#include "Sys_hdr.h"
#include "World.h"
#include "Sv_net.h"

/*
======================================
Expose to system
======================================
*/
namespace VoidNet
{
	bool InitNetwork();
	void ShutdownNetwork();
}


class CServer : public CNetServer,
				public I_ConHandler
{
public:
	CServer();
	~CServer();

	void RunFrame();

	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	bool Init();
	void Shutdown();
	void Restart();

	void LoadWorld(const char * mapname);
	void PrintServerStatus();

	//=================================================
	
	world_t	*	m_pWorld;
	bool		m_active;

	//=================================================
	//CVars
	CVar	m_cDedicated;
	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir

};


#endif