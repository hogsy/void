#ifndef VOID_CLIENT_COMMANDS
#define VOID_CLIENT_COMMANDS

#include "Sys_hdr.h"
#include "In_defs.h"

//======================================================================================
//======================================================================================

#define CL_CMDBUFFERSIZE	8

//Registered Command Ids

#define CMD_MOVE_FORWARD	0		
#define CMD_MOVE_BACKWARD	1
#define CMD_MOVE_LEFT		2
#define CMD_MOVE_RIGHT		3
#define CMD_ROTATE_LEFT		4
#define CMD_ROTATE_RIGHT	5
#define CMD_ROTATE_UP		6
#define CMD_ROTATE_DOWN		7
#define CMD_BIND			8
#define CMD_BINDLIST		9
#define CMD_UNBIND			10
#define CMD_UNBINDALL		11
#define CMD_CAM				12

//======================================================================================
//======================================================================================
/*
=========================================
List of all the Keys and a pointer to 
the function they are bound to
=========================================
*/
struct ClientKey
{
	ClientKey()	{ szCommand = 0; pCmd = 0;	}
	~ClientKey(){ if(szCommand)	delete [] szCommand; pCmd = 0; }
	
	char *	   szCommand;
	CCommand * pCmd;
};

//======================================================================================
//======================================================================================
/*
======================================
Client Command Handler
-handle input events
-maintain a list of all the valid commands
-maintain a command buffer that gets executed every frame
-provide means to register new commands
-provide means to bind keys to commands
======================================
*/

class CClientCmdHandler : public I_InKeyListener
{
public:

	CClientCmdHandler(CClient * pclient);
	~CClientCmdHandler();

	void SetListenerState(bool on);
	void RunCommands();

	void HandleKeyEvent	  (const KeyEvent_t &kevent);
	void HandleCursorEvent(const float &ix,
						   const float &iy,
					       const float &iz);

	void BindFuncToKey(int argc, char** argv);
	void Unbind(int argc, char** argv);
	void BindList();
	void Unbindall();

private:

	void AddToCmdBuffer(ClientKey * const pcommand);
	void RemoveFromCmdBuffer(const ClientKey * pcommand);

	CClient   * m_pClient;
	ClientKey	m_cmdKeys[IN_NUMKEYS];
	ClientKey * m_cmdBuffer[CL_CMDBUFFERSIZE];
};


/*
=========================================
256 item array signifying all key constants
whenever a key is pressed, the client Handlekeys function looks
through this array to see if there is a function bound to that key, and 
executes it if there is.
=========================================
*/
extern ClientKey *  m_clientkeys;			

/*
=========================================
command buffer which gets executed each frame
and is filled/emptied according to input
=========================================
*/
extern ClientKey ** m_commandbuffer;		

/*
============================================================================
A Constant list of Special keys "names"
and their corresponding values
only 
============================================================================
*/
struct ClientKeyConstants_t
{
	const char * key;
	unsigned int val;
};

const ClientKeyConstants_t keytable[] =
{
	{	"MOUSE1",		INKEY_MOUSE1	},
	{	"MOUSE2",		INKEY_MOUSE2	},
	{	"MOUSE3",		INKEY_MOUSE3	},
	{	"MOUSE4",		INKEY_MOUSE4	},
	{	"UPARROW",		INKEY_UPARROW	},
	{	"DOWNARROW",	INKEY_DOWNARROW	},
	{	"LEFTARROW",	INKEY_LEFTARROW	},
	{	"RIGHTARROW",	INKEY_RIGHTARROW},
	{	"TAB",			INKEY_TAB		},
	{	"ESC",			INKEY_ESCAPE	},
	{	"F1",			INKEY_F1	},
	{	"F2",			INKEY_F2	},
	{	"F3",			INKEY_F3	},
	{	"F4",			INKEY_F4	},
	{	"F5",			INKEY_F5	},
	{	"F6",			INKEY_F6	},
	{	"F7",			INKEY_F7	},
	{	"F8",			INKEY_F8	},
	{	"F9",			INKEY_F9	},
	{	"F10",			INKEY_F10	},
	{	"F11",			INKEY_F11	},
	{	"F12",			INKEY_F12	},
	{	"INS",			INKEY_INS	},
	{	"DEL",			INKEY_DEL	},
	{	"HOME",			INKEY_HOME	},
	{	"END",			INKEY_END	},
	{	"PGUP",			INKEY_PGUP	},
	{	"PGDN",			INKEY_PGDN	},
	{	0,	0}
};


#endif