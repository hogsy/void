#ifndef CL_MAIN
#define CL_MAIN

#include "Sys_hdr.h"
#include "Net_defs.h"
#include "Net_util.h"
#include "Net_sock.h"
#include "I_hud.h"

#define CL_NUMBUTTONS 4

/*
============================================================================
List of all the Keys and what command 
do they add to the command buffer

whenever a key is pressed,
the client Handlekeys function looks
through this array to see if there is a 
command associated with the key, and if there is
the command is entered to the command buffer
to be processed later that frame

if there isnt, we just send it to the console
as there might be a console function associated with it
============================================================================
*/

typedef void (*CL_FUNC)();

class cl_keys
{
public:
	cl_keys()
	{ 	everyframe=false;
		command=0;
		func = 0;
	 }
	
	~cl_keys()
	{
		if(command !=0)
			delete [] command;
		func = 0;
	}

	CL_FUNC func;
	char *	command;
	bool	everyframe;
};


/*
=====================================
This what the client updates and sends
to the server
=====================================
*/
class CCmd
{
	eyepoint_t origin;

	short buttons[CL_NUMBUTTONS];

	float forwardmove;
	float sidemove;
	float upmove;
};


/*
=====================================
The client
=====================================
*/


class CClient
{
public:
	CClient();
	~CClient();

	bool InitNet();
	bool CloseNet();

	bool InitGame();//sector_t *sec);
	bool ShutdownGame();

	void Cl_WriteBindTable(FILE *fp);
	
	bool ConnectTo(char *ipaddr, int port);
	bool Disconnect();

	bool LoadWorld(world_t *world=0);
	bool UnloadWorld();

	
	//run local stuff, 
	//messages received from the server would be handled here
	void RunFrame();

	static bool Name(const CVar * var, int argc, char** argv);
	
	//Console funcs
	static void Bind(int argc, char** argv);
	static void BindList(int argc, char** argv);
	static void Unbindall(int argc, char** argv);
	static void Unbind(int argc, char** argv);
	static void CamPath(int argc,char **argv);

	//input
	friend void ClientHandleCursor(const float &x, const float &y, const float &z);
	friend void ClientHandleKey(const KeyEvent_t *kevent);
	
	friend void Talk(int argc,char **argv);

/*
	friend void ClMoveForward();
	friend void ClMoveBackward();
	friend void ClMoveRight();
	friend void ClMoveLeft();
	friend void ClKRotateRight();
	friend void ClKRotateLeft();
	friend void ClKRotateUp();
	friend void ClKRotateDown();
*/
	friend void MoveForward(int argc, char** argv);
	friend void MoveBackward(int argc, char** argv);
	friend void MoveRight(int argc, char** argv);
	friend void MoveLeft(int argc, char** argv);
	friend void KRotateRight(int argc, char** argv);
	friend void KRotateLeft(int argc, char** argv);
	friend void KRotateUp(int argc, char** argv);
	friend void KRotateDown(int argc, char** argv);

	friend void Connect(int argc, char** argv);
	
	eyepoint_t  eye;

	unsigned int	m_recvseq;		//packet num
	CNBuffer		m_recvBuf;		//network buffer we read from

	unsigned int	m_sendseq;		//packet num
	CNBuffer		m_sendBuf;		//network buffer we write to
	
	bool m_active;
	bool m_connected;
	bool m_ingame;

	world_t    *m_world;

	void RotateRight(float val=5.0);
	void RotateLeft(float val=5.0);
	void RotateUp(float val=5.0);
	void RotateDown(float val=5.0);

	void Move(vector_t *dir, float time);

	static	cl_keys * m_clientkeys;

private:

	
	
	
	static	CVar *		m_clport;
	static  CVar *		m_clname;
	static  CVar *		m_clrate;
	static  CVar *		m_noclip;

	static	I_RHud *	m_rHud;



	CSocket	 	m_sock;


//	char		m_ipaddr[16];
	
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