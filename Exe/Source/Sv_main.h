#ifndef VOID_NETWORK_SERVER
#define VOID_NETWORK_SERVER

#include "Sys_hdr.h"

/*
==========================================
Public defs
==========================================
*/
namespace VoidNet
{
	bool InitNetwork();
	void ShutdownNetwork();
}

/*
==========================================
Private defs
==========================================
*/
namespace VoidServer
{
}

/*
==========================================
Game independent network server
==========================================
*/
class CServer : public I_CVarHandler,
				public I_CmdHandler
{
public:

	CServer();
	~CServer();

	bool Init();
	void Shutdown();

	void RunFrame();

	//CVar Handler
	bool HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs);

	//Cmd Handler
	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);

private:

	CVar	m_cPort;		//Listen port
	CVar 	m_cHostname;	//Hostname
	CVar 	m_cMaxClients;	//Max Clients
	CVar	m_cGame;		//Game Dir
};




//======================================================================================
//======================================================================================





#if 0

#include "Sys_hdr.h"
#include "Net_defs.h"
#include "Net_util.h"
#include "Sv_client.h"


/*
=====================================
Manage Client Connections
=====================================
*/

class CNetClients
{
public:
	CNetClients();
	//void Init(int numclients, int svport);
	void Init(SOCKADDR_IN &addr,int numclients, int svport);
	
	void Shutdown();
	CSVClient * GetFreeClient();

	void WriteTalkMessage(char *msg, int numSender, int numReceiver =0);
	char * PrintClInfo();		//number of clients and names
	char * PrintClStatus();		//client frags,pings, skins ?

	void RunClients();

	int m_maxclients;
	int m_curclients;
	
	//dynamic client info
	CSVClient *m_svclients;		
};


/*
=====================================
the Server
=====================================
*/


class CServer
{
public:
	CServer();
	~CServer();
	
	bool Init();

//	bool InitGame(world_t *world);
	bool InitGame(char *mapname);
	bool Shutdown();

	void RunFrame();
	
	//used by loopback client/servers
	world_t * GetWorld() {return m_pWorld;}

	bool	m_active;	//is the server active 
	double	m_time;		//time since its active
	char	m_mapname[64];

	//exposed for the svc_client objects
//	bool SendInfoPacket(SOCKADDR_IN *addr);		//brief server info
//	bool SendStatusPacket(SOCKADDR_IN *addr);	//detail status information
	
	void WriteInfoMsg(CNBuffer *dest);
	void WriteStatusMsg(CNBuffer *dest);
	bool WritePlayerInfo(CNBuffer *dest,int playernum);

	static void SV_Status(int argc,char **argv);

private:
	
	//CVars
	static CVar	*		m_port;
	static CVar *		m_hostname;
	static CVar *		m_dedicated;
	static CVar *		m_maxclients;
	static CVar	*		m_game;
	
	class CConnectQ
	{
	public:
		CConnectQ() {challenge = 0; inuse = false; lastmsg = 0.0f;}
		char	ipaddr[16];
		int		challenge;
		bool	inuse;
		float	lastmsg;
	};
	CConnectQ	m_connectq[MAX_CONNECTQ];

	world_t    *m_pWorld;
	
	int			m_protocolversion;
	
//	char		m_ipaddr[16];
//	char		m_computername[256];
	int			m_totalconnections;
		
	//Listener socket stuff
	SOCKET				m_socket;				//listener socket
	SOCKADDR_IN			m_addr;					//socket addr list
	WSAEVENT			m_event;				//Event object for nonblocking more
	LPWSANETWORKEVENTS  m_pNetworkEvents;		//Event info
	SOCKADDR_IN			m_raddr;				//client query connections
	CNBuffer			m_sockBuf;				//listener socket buffer


	CNBuffer			m_datagram;				//cleared and sent each frame to each client
	CNBuffer			m_reliable_datagram;	//copied to all clients at end of frame

	void CheckNewConnections();
	bool SendNBuffer(SOCKADDR_IN * addr);

	//helper function
	bool SendRejectPacket(SOCKADDR_IN *addr, char *reason);	
	bool SendAcceptPacket(SOCKADDR_IN *addr, int port);
};

extern CServer * g_pServer;

#endif


#endif
