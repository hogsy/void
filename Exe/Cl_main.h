#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "I_renderer.h"
#include "Net_defs.h"
#include "Net_chan.h"
#include "Snd_defs.h"

//======================================================================================
//======================================================================================

namespace VoidClient
{
	class CClientCmdHandler;
//	class CClientNet;
}

/*
=====================================
Client class
-controls the camera
-the game commands,
-the hud while IN GAME
-processes server messages to update world
-predicts item positions
-basically all the user interactive elements which are only available when in game
=====================================
*/
class CClient :	public I_CmdHandler,
				public I_CVarHandler
{
public:
	CClient(I_Renderer * prenderer);
	~CClient();

	void RunFrame();
	
	//Command Handler Interface
	void HandleCommand(HCMD cmdId, const CParms &parms);

	//CVar Handler Interface
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	
	void SetInputState(bool on);
	void WriteBindTable(FILE *fp);

private:
	void HandleSpawnParms();
	void HandleOOBMessage();

	//==================================================
	//Console commands
	void ConnectTo(const char * ipaddr);
	void Disconnect();

	//Movement
	void Move(vector_t *dir, float time);
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void RotateRight(float val=5.0);
	void RotateLeft(float val=5.0);
	void RotateUp(float val=5.0);
	void RotateDown(float val=5.0);

	void CamPath();
	void Talk(const char * msg);

//	bool LoadWorld(world_t * world);
	bool LoadWorld(const char *worldname);
	bool UnloadWorld();

	//==================================================
	//Network Specific Stuff

	//Read any waiting packets
	void ReadPackets();	
//	void CheckResends();	

	//Send a connection request wtih a challenge number
	void SendConnectReq();
	void SendConnectParms();

	CNetBuffer   m_buffer;
	CNetChan	 m_netChan;
	VoidNet::CNetSocket * m_pSock;
	
	char		m_svServerAddr[24];
	bool		m_bLocalServer;
	float		m_fNextConReq;	
	int			m_challenge;
	const char* m_szLastOOBMsg;	//Keep Track of the last OOB message sent

	int			m_state;

	//==================================================
	//Client CVars
	CVar		m_clport;
	CVar 		m_clname;
	CVar 		m_clrate;
	CVar 		m_noclip;

	//Command Handler
	friend class VoidClient::CClientCmdHandler;
	VoidClient::CClientCmdHandler * m_pCmdHandler;

	//Renderer and HUD interfaces
	I_Renderer* m_pRender;
	I_RHud    *	m_pHud;

	bool m_ingame;

	//==================================================
	//the following should be accessible by the game dll
	
	eyepoint_t  eye;
	vector_t	desired_movement;
	
	int		m_campath;
	float	m_camtime;
	float	m_acceleration;
	float	m_maxvelocity;

	void Spawn(vector_t	*origin, vector_t *angles);

	//==================================================
	//Client side stuff
	int		m_hsTalk;		//handle to talk sound
};

#endif