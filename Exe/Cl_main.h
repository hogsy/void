#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "I_renderer.h"

namespace VoidClient
{
	class CClientCmdHandler;
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
class CClient :	public I_CmdHandler 
{
public:
	CClient();
	~CClient();
	
	bool LoadWorld(world_t * world);
	bool UnloadWorld();

	void WriteBindTable(FILE *fp);
	
	void SetInputState(bool on);

	//Command Handler Interface
	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);

	//CVar Handler Interface
//	bool HandleCVar(const CVar * cvar, int numArgs, char ** szArgs);

	//run local stuff, 
	//messages received from the server would be handled here
	void RunFrame();

private:

	bool m_active;
	bool m_connected;
	bool m_ingame;

	eyepoint_t  eye;

	bool ConnectTo(char *ipaddr, int port);
	bool Disconnect();

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

	//Console funcs
	void CamPath(int argc,char **argv);

	CVar	m_clport;
	CVar 	m_clname;
	CVar 	m_clrate;
	CVar 	m_noclip;

//	world_t    *m_pWorld;

	friend class VoidClient::CClientCmdHandler;
	VoidClient::CClientCmdHandler * m_pCmdHandler;

	I_RHud *	m_rHud;

//	CSocket	 	m_sock;
	
	char		m_svipaddr[16];		//addr we are currently connected to
	int			m_svport;


	vector_t	desired_movement;

	void RegCommands();		//register commmands
	
	//the following should be accessible by the game dll
	
	int		m_campath;
	float	m_camtime;
	float	m_acceleration;
	float	m_maxvelocity;

	void Spawn(vector_t	*origin, vector_t *angles);
};

#endif