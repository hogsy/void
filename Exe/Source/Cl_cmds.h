#ifndef VOID_CLIENT_COMMANDS
#define VOID_CLIENT_COMMANDS

#define CL_CMDBUFFERSIZE	8

#include "Sys_hdr.h"
#include "In_defs.h"


class CClientCommandHandler //: public I_InKeyListener
{
//	void HandleKeyEvent	(const KeyEvent_t &kevent) { }
/*	void HandleCursorEvent(const float &ix,
					   const float &iy,
					   const float &iz){}
*/
};

/*
=========================================
List of all the Keys and a pointer to 
the function they are bound to
=========================================
*/
struct ClientKey
{
	ClientKey()
	{ 	
		pFunc = 0;
		szCommand = 0;
		bBuffered = false;
	}
	
	~ClientKey()
	{
		if(szCommand) delete szCommand;
		pFunc = 0;
	}

	CFUNC    pFunc;		//Function the key is bound to
	char   * szCommand;	//The Command String
	bool	 bBuffered;	//Is the function supposed to be added to the command buffer ?
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
typedef struct clkeynames_s
{
	const char		*key;
	unsigned int	val;

}keyconstants_t;

const keyconstants_t keytable[] =
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
