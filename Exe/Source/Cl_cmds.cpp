
/*
======================================
Client Command Handling

-maintain a list of all the valid commands
-maintain a command buffer that gets executed every frame
-provide means to register new commands
-provide means to bind keys to commands
======================================
*/

#include "Cl_main.h"
#include "In_defs.h"

/*
============================================================================
A Constant list of Special keys "names"
and their corresponding values
only 
============================================================================
*/

typedef struct	conskeys
{
	const char		*key;
	unsigned int	val;
}keyvals_t;


const keyvals_t keytable[] =
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


//cl_keys_t  CClient::m_clientkeys[256];
cl_keys  * CClient::m_clientkeys;


/*
commands starting with a + will not be removed from the command buffer
other commands are just entered once on keydown events
until the same command is entered with another + or -
*/

//============================================================================

/*
======================================
Handle keys pressed this frame
======================================
*/

void ClientHandleKey(const KeyEvent_t *kevent)
{
	if(g_pClient->m_clientkeys[(kevent->id)].command)
	{
//FIX ME
		//if its every frame, then add to buffer 
		if((((g_pClient->m_clientkeys[(kevent->id)]).everyframe==true) && (kevent->state != BUTTONUP)) ||
		    (kevent->state == BUTTONDOWN))
		{
			g_pCons->ExecString((g_pClient->m_clientkeys[(kevent->id)]).command);
		}
	}
}

/*
======================================
Filter and handle mouse input
======================================
*/

void ClientHandleCursor(const float &x, const float &y, const float &z)
{
	g_pClient->RotateRight(x);
	g_pClient->RotateUp(y);
}





/*
======================================
Console command, bind something
======================================
*/

void CClient::Bind(int argc, char** argv)
{
	//no arguments
	if(argc==1)
	{
		ComPrintf("bind usage - <bind> <command> ...\n");
		return;
	}

	unsigned char keynum= 0;

	//find keyname i.e LEFT_ARROW etc
	if(strlen(argv[1]) > 1)
	{
		for(int x=0;keytable[x].key;x++)
		{
			if(!stricmp(keytable[x].key,argv[1]))
			{
				keynum = keytable[x].val;
				break;
			}
		}
	}
	else
		keynum = argv[1][0];
		
	//make sure that the keynum is valid
	if((keynum <= 0) || (keynum > 255))
	{
		ComPrintf("bind - error %s is not a valid key -%d\n",argv[1],keynum);
		return;
	}

	//only two args, just show binding and return
	if(argc == 2)
	{
		if(m_clientkeys[keynum].command)
			ComPrintf("\"%s\" = \"%s\"\n",argv[1],m_clientkeys[keynum].command);
		else
			ComPrintf("\"%s\" = \"\"\n",argv[1]);
		return;
	}

	int size=1;
	for(int i=2;i<argc;i++)
	{	size += (strlen(argv[i])+1);
	}

	if(m_clientkeys[keynum].command)
		delete [] m_clientkeys[keynum].command;

	m_clientkeys[keynum].command = new char[size];
	memset(m_clientkeys[keynum].command,0,size);

	char *p=m_clientkeys[keynum].command;
	for(int x=2;x<=argc;x++)
	{
		char *c = argv[x];
		while(*c && * c!='\0')
		{
			*p=*c;
			c++;
			p++;
		}
		//no more strings, break
		if((x+1)>=argc)
		{
			*p = '\0';
			break;
		}
		//add a space
		*p = ' ';
		p++;
	}

	if(argv[2][0] == '+' || argv[2][0] == '-')
		m_clientkeys[keynum].everyframe = true;
	else
		m_clientkeys[keynum].everyframe = false;

	ComPrintf("\"%s\"(%d) = \"%s\"\n",argv[1],keynum,m_clientkeys[keynum].command);
}	


/*
======================================
Runs all the commands in the 
command buffer for the frame
======================================
*/
void CClient::RunCommands()
{
}

/*
======================================
lists all our current bindings
======================================
*/

void CClient::BindList(int argc, char** argv)
{
	ComPrintf("Client Binding List\n");
	ComPrintf("===================\n");
	
	for(unsigned int i=0;i<256;i++)
	{
		if(m_clientkeys[i].command)
		{
			char key[16];
			bool hit=false;
			
			for(int x=0;keytable[x].key;x++)
			{
				if(keytable[x].val == i)
				{
					strcpy(key,keytable[x].key);
					hit = true;
					break;
				}
			}
			if(!hit)
			{
				memset(key,'\0',sizeof(key));
				key[0] = i;
			}
			ComPrintf("\"%s\" = \"%s\"\n",key,m_clientkeys[i].command);
		}
	}
}

/*
======================================
unbind all our bindings
======================================
*/
void CClient::Unbindall(int argc, char** argv)
{
	for(int i=0;i<256;i++)
	{
		if(m_clientkeys[i].command)
		{
			delete [] m_clientkeys[i].command;
			m_clientkeys[i].command=0;
		}
	}
	ComPrintf("CClient::Unbindall - OK\n");

}

/*
======================================
unbind all our bindings
======================================
*/
void CClient::Unbind(int argc, char** argv)
{
	if(argc <2)
	{
		ComPrintf("Usage - <unbind> <keyname>\n");
		return;
	}

	unsigned char keynum= 0;

	//not a special KEY
	if(strlen(argv[1]) > 1)
	{
		for(int x=0;keytable[x].key;x++)
		{
			if(!stricmp(keytable[x].key,argv[1]))
			{
				keynum = keytable[x].val;
				break;
			}
		}
	}
	else
		keynum = argv[1][0];
		
	//make sure that the key is valid
	if((keynum == 0) || (keynum > 255))
	{
		ComPrintf("bind - error %s is not a valid key -%d\n",argv[1],keynum);
		return;
	}

	if(m_clientkeys[keynum].command)
	{
		delete [] m_clientkeys[keynum].command;
		m_clientkeys[keynum].command=0;
	}
	ComPrintf("\"%s\" = \"\"\n",argv[1]);
}



/*
======================================
Register the EXE Client Commands
======================================
*/

void CClient::RegCommands()
{
	g_pCons->RegisterCFunc("bind", &Bind);
	g_pCons->RegisterCFunc("unbind", &Unbind);
	g_pCons->RegisterCFunc("bindlist", &BindList);
	g_pCons->RegisterCFunc("unbindall", &Unbindall);
	g_pCons->RegisterCFunc("+forward",&MoveForward);
	g_pCons->RegisterCFunc("+back",&MoveBackward);
	g_pCons->RegisterCFunc("+moveleft",&MoveLeft);
	g_pCons->RegisterCFunc("+moveright",&MoveRight);
	g_pCons->RegisterCFunc("+right",&KRotateRight);
	g_pCons->RegisterCFunc("+left",&KRotateLeft);
	g_pCons->RegisterCFunc("+lookup",&KRotateUp);
	g_pCons->RegisterCFunc("+lookdown",&KRotateDown);
	g_pCons->RegisterCFunc("cam", &CamPath);
}


/*
======================================
Writes the current Bind table to a config file
called from the Console Shutdown func
======================================
*/

void CClient::Cl_WriteBindTable(FILE *fp)
{
	for(unsigned int i=0;i<256;i++)
	{
		//there is a binding for this key
		if(m_clientkeys[i].command != 0)
		{
			char line[80];
			bool hit=false;
			
			strcpy(line,"bind ");
			for(unsigned int x=0;keytable[x].val;x++)
			{
				if(keytable[x].val == i)
				{
					strcat(line,keytable[x].key);
					hit = true;
					break;
				}
			}

			if(!hit)
			{
				char p[2];
				p[0] = i;
				p[1] = '\0';
				strcat(line,p);
			}
			
			strcat(line," ");
			strcat(line, m_clientkeys[i].command);
			strcat(line,"\n");
			fputs(line,fp);
		}
	}
}


/*
===========
follow a camera path
===========
*/
extern world_t		 *g_pWorld;
void CClient::CamPath(int argc,char **argv)
{
	// find the head path node
	for (int ent=0; ent<g_pWorld->nentities; ent++)
	{
		if (strcmp(key_get_value(g_pWorld, ent, "classname"), "misc_camera_path_head") == 0)
		{
			g_pClient->m_campath = ent;
			g_pClient->m_camtime = g_fcurTime;

			vector_t origin;
			key_get_vector(g_pWorld, ent, "origin", origin);
			VectorCopy(origin, g_pClient->eye.origin); // move to first point of path

			return;
		}
	}
}


