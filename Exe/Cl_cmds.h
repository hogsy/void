#ifndef VOID_CLIENT_COMMANDS
#define VOID_CLIENT_COMMANDS

#include "In_defs.h"


//Registered Command Ids
enum
{
	CMD_MOVE_FORWARD  = 0,
	CMD_MOVE_BACKWARD =	1,
	CMD_MOVE_LEFT	  = 2,
	CMD_MOVE_RIGHT	  = 3,
	CMD_ROTATE_LEFT	  =	4,
	CMD_ROTATE_RIGHT  =	5,
	CMD_ROTATE_UP	  =	6,
	CMD_ROTATE_DOWN	  =	7,
	CMD_BIND		  =	8,
	CMD_BINDLIST	  =	9,
	CMD_UNBIND		  =	10,
	CMD_UNBINDALL	  =	11,
	CMD_CAM			  =	12,
	CMD_TALK		  = 13,
	CMD_DEBUG		  = 14
};

/*
================================================
Utility struct to easily add game commands
================================================
*/
struct ClientGameCmd
{
	const char * szCmd;
	unsigned int id;
};
const ClientGameCmd g_clGameCmds[] =
{
	{	"+forward",		CMD_MOVE_FORWARD  },
	{	"+back",		CMD_MOVE_BACKWARD },
	{	"+moveleft",	CMD_MOVE_LEFT	},
	{	"+moveright",	CMD_MOVE_RIGHT	},
	{	"+right",		CMD_ROTATE_RIGHT},
	{	"+left",		CMD_ROTATE_LEFT },
	{	"+lookup",		CMD_ROTATE_UP	},
	{	"+lookdown",	CMD_ROTATE_DOWN },
	{	0, 0}
};

/*
=========================================
List of all the Keys and a pointer to 
the function they are bound to
=========================================
*/
struct CCommand;

struct ClientKey
{
	ClientKey()	{ pCmd = 0;	szCommand[0] = 0;}
	~ClientKey(){ pCmd = 0; } 
	
	char 	   szCommand[80];
	CCommand * pCmd;
};

/*
=========================================
A Constant list of Special keys "names"
and their corresponding values only 
=========================================
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
const int CL_CMDBUFFERSIZE = 8;

class CClientGameInput : public I_InKeyListener,	
						 public I_InCursorListener
{
public:

	CClientGameInput();
	~CClientGameInput();

	void IntializeBinds();

	//Call this to execute all the commands buffered during the frame
	void RunCommands();
	
	//Will update cursor pos if it had changed and return true
	bool CursorChanged() const { return m_bCursorChanged; }
	void UpdateCursorPos(float &ix, float &iy, float &iz);

	void HandleKeyEvent	  (const KeyEvent &kevent);
	void HandleCursorEvent(const float &ix,
						   const float &iy,
					       const float &iz);

	void BindFuncToKey(const CParms &parms, bool bPrint = true);
	void Unbind(const CParms &parms);
	void Unbindall();
	void BindList() const;

	void ExecBindsFile(const char * szBindsfile);
	void WriteBinds(const char * szBindsfile);

private:

	void AddToCmdBuffer(ClientKey * const pcommand);
	void RemoveFromCmdBuffer(const ClientKey * pcommand);

	float	m_fXpos, m_fYpos, m_fZpos;
	bool	m_bCursorChanged;

	CParms	m_Parms;

	ClientKey	m_cmdKeys[IN_NUMKEYS];
	ClientKey * m_cmdBuffer[CL_CMDBUFFERSIZE];
};



#endif