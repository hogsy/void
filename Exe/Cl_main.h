#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "I_renderer.h"
#include "Snd_defs.h"

/*
======================================
Predeclarations
======================================
*/
class CClientCmdHandler;	//Handles all client command processing
class CClientNetHandler;	//Handles all client network communication

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

	enum ClMsgType
	{
		DEFAULT,
		SERVER_MESSAGE,
		TALK_MESSAGE
	};

	void Print(ClMsgType type, const char * msg, ...);

	bool LoadWorld(const char *worldname);
	bool UnloadWorld();

	//==================================================
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

	//==================================================
	//Console commands
	void Talk(const char * string);
	bool ValidateName(const CParms &parms);
	bool ValidateRate(const CParms &parms);
	void CamPath();

	//==================================================
	//Client CVars
	CVar	m_clport;
	CVar	m_clname;
	CVar	m_clrate;
	CVar	m_noclip;

	//==================================================
	//Subsystems
	friend class CClientCmdHandler;
	friend class CClientNetHandler;

	CClientCmdHandler * m_pCmdHandler;
	CClientNetHandler * m_pClNet;

	//Renderer and HUD interfaces
	I_Renderer* m_pRender;
	I_RHud    *	m_pHud;

	float		m_fFrameTime;

	//==================================================
	//Client side stuff
	int		m_hsTalk;		//handle to talk sound
	int		m_hsMessage;	//handle to server message sound

	//==================================================
	//the following should be accessible by the game dll

	bool		m_ingame;
	eyepoint_t  eye;
	vector_t	desired_movement;
	
	int			m_campath;
	float		m_camtime;
	float		m_acceleration;
	float		m_maxvelocity;

	void Spawn(vector_t	*origin, vector_t *angles);
};

#endif