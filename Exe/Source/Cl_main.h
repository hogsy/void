#ifndef VOID_CLIENT_MAIN
#define VOID_CLIENT_MAIN

#include "Sys_hdr.h"
#include "Net_defs.h"
#include "Net_util.h"
#include "Net_sock.h"
#include "I_hud.h"
#include "In_defs.h"


/*
=====================================
The client
=====================================
*/
class CClient : public I_InCursorListener, 
				public I_InKeyListener,
				public I_CmdHandler
{
public:
	CClient();
	~CClient();

	bool InitNet();
	bool CloseNet();

	bool InitGame();
	bool ShutdownGame();

	void WriteBindTable(FILE *fp);
	
	bool ConnectTo(char *ipaddr, int port);
	bool Disconnect();

	bool LoadWorld(world_t *world=0);
	bool UnloadWorld();

	void SetInputState(bool on);

	//Input Listener Interface
	void HandleKeyEvent	(const KeyEvent_t &kevent);
	void HandleCursorEvent(const float &ix,
						   const float &iy,
						   const float &iz);

	//Command Handler Interface
	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);

	//run local stuff, 
	//messages received from the server would be handled here
	void RunFrame();

	friend void Talk(int argc,char **argv);
	friend void Connect(int argc, char** argv);
	
	bool m_active;
	bool m_connected;
	bool m_ingame;

	eyepoint_t  eye;

private:

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
	void BindFuncToKey(int argc, char** argv);
	void BindList(int argc, char** argv);
	void Unbindall(int argc, char** argv);
	void Unbind(int argc, char** argv);
	void CamPath(int argc,char **argv);


	
	static	CVar *		m_clport;
	static  CVar *		m_clname;
	static  CVar *		m_clrate;
	static  CVar *		m_noclip;


	unsigned int	m_recvseq;		//packet num
	CNBuffer		m_recvBuf;		//network buffer we read from

	unsigned int	m_sendseq;		//packet num
	CNBuffer		m_sendBuf;		//network buffer we write to

	world_t    *m_world;

	friend class CClientCmdHandler;
	CClientCmdHandler * m_pCmdHandler;

//	CClientCommandHandler * m_pCmdHandler;

	I_RHud *	m_rHud;


	CSocket	 	m_sock;
	
	char		m_svipaddr[16];		//addr we are currently connected to
	int			m_svport;


	vector_t	desired_movement;

	void RegCommands();		//register commmands
	void RunCommands();		//exec the commands in the command buffer
	
	//the following should be accessible by the game dll
	
	int		m_campath;
	float	m_camtime;
	float	m_acceleration;
	float	m_maxvelocity;

	void Spawn(vector_t	*origin, vector_t *angles);
};


extern CClient * g_pClient;

#endif