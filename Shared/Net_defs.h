#ifndef NETWORK_DEFS_HEADER
#define NETWORK_DEFS_HEADER

/*
======================================
Network Client states
======================================
*/
enum
{	
	CL_FREE = 0,		//nothing doing. can be used
	CL_INUSE = 1,		//trying to connect
	CL_CONNECTED = 2,	//havent spawned yet
	CL_SPAWNED = 4		//in game
};

/*
======================================
Other misc shared definitions
======================================
*/
const char szWORLDDIR[]     = "Worlds/";

const int  SV_DEFAULT_PORT = 20010;
const int  CL_DEFAULT_PORT = 20011;

const int  SV_MAX_CLIENTS = 16;

const int  PACKET_HEADER	  =	8;
const int  MAX_DATAGRAM_SIZE  =	1450;
const int  MAX_BUFFER_SIZE	  = 2900;

const int  SV_TIMEOUT_INTERVAL= 10;			//10 seconds of silence

/*
======================================
Stats of a Network comm channel
======================================
*/
struct NetChanState
{
	NetChanState() { Reset(); }
	void Reset() 
	{
		inMsgId = inAckedId = outMsgId = lastOutReliableId = 0;
		dropCount = goodCount = numChokes = 0;
	}

	uint	inMsgId;				//Latest incoming messageId
	uint	inAckedId;				//Latest remotely acked message.
	uint	outMsgId;				//Outgoing messageId
	uint	lastOutReliableId;		//Id of the last reliable message sent
	int		dropCount;				//Number of packets that went missing
	int		goodCount;				//Number of packed received okay
	int		numChokes;				//Times we throttled back to conform to the rate
};

/*
======================================
Server State Struct
======================================
*/
struct ServerState
{
	ServerState()
	{	
		port = 0;
		maxClients = numClients = levelId = 0;
		memset(hostName,0,sizeof(hostName));
		memset(gameName,0,sizeof(gameName));
		memset(worldname,0,sizeof(worldname));
		memset(localAddr,0,sizeof(localAddr));
	}

	int		maxClients;		//Max num of clients
	int		numClients;		//Current num of clients
	
	char	worldname[32];	//Map name
	int		levelId;		//Current map id
	
	char	localAddr[24];	//Server address
	short	port;			//Server port num
	
	char	hostName[32];	//Server name
	char	gameName[32];	//Game name
};

/*
======================================
The Network library needs access to
game/frame time variables
======================================
*/
namespace System
{
	extern float	g_fframeTime;		//Frametime
	extern float	g_fcurTime;			//Current Timer
}

#endif