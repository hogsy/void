#ifndef INC_CLIENT_INTERFACE
#define INC_CLIENT_INTERFACE

//exposed to the network dll 
//to the game dll as well ?
#include "Com_buffer.h"

enum ClMsgType
{
	CLMSG_DEFAULT,
	CLMSG_SERVER,
	CLMSG_TALK
};

struct ClUserInfo
{
	ClUserInfo() { strcpy(name,"Player"); rate = 2500; vport = 0; }

	char	name[32];
	int		rate;
	short	vport;
};


struct I_ClientNetHandler
{
	//Client print func
	virtual void Print(ClMsgType type, const char * msg, ...)=0;

	//Parse and handle a game message
	virtual void HandleGameMsg(CBuffer &buffer)=0; 
	
	//Parse and handle spawm parms
	virtual void HandleSpawnMsg(const byte &msgId, CBuffer &buffer)=0; 
	
	//Handle disconnect from server
	virtual void HandleDisconnect(bool listenserver)=0;

	virtual const ClUserInfo & GetUserInfo() const = 0;
};

#endif