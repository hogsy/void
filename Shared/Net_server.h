#ifndef  VOID_NETWORK_SERVER
#define  VOID_NETWORK_SERVER

#include "Com_buffer.h"
#include "Net_defs.h"

/*
============================================================================
The Game server usually needs to know how to handle a client disconnection
For example the game server shouldnt broadcast a Client disconnection
if it was the Server which went down.

The OnClientDrop function includes a EDisconnectReason parameter.
============================================================================
*/
enum EDisconnectReason
{
	SERVER_QUIT=0,
	CLIENT_QUIT=1,
	CLIENT_TIMEOUT=2,
	CLIENT_OVERFLOW=3,
	CLIENT_BADMSG = 4
};

/*
============================================================================
This info is sent to the client in a series of stages during the
connection phase. Giving the client indexes for Images/Sounds/Models and
Entity baselines will save a LOT of network traffic during gameplay.

Therefor the Game Server NEEDs to update this data on every map change.

The struct is HUGE in size, About 22k, 
But making a list doesnt seem worth the hassle
============================================================================
*/
struct NetSignOnBufs
{
	enum
	{
		MAX_IMAGE_BUFS = 4
		MAX_MODEL_BUFS = 4
		MAX_SOUND_BUFS = 4
		MAX_ENTITY_BUFS = 4
	};

	NetSignOnBufs() 
	{
		numImageBufs = 0;
		numModelBufs = 0;
		numSoundBufs = 0;
		numEntityBufs = 0;
	}

	CBuffer  gameInfo;

	int		 numImageBufs;
	CBuffer  imageList[MAX_IMAGE_BUFS];
	
	int      numModelBufs;
	CBuffer  modelList[MAX_MODEL_BUFS];

	int		 numSoundBufs;
	CBuffer  soundList[MAX_SOUND_BUFS];

	int      numEntityBufs;
	CBuffer  entityList[MAX_ENTITY_BUFS];
};


/*
============================================================================
This callback interface should be implemented by the main server class so
that the Network server can let the main server handle special events
============================================================================
*/
struct I_Server
{
	//process ingame client message
	virtual void HandleClientMsg(int clNum, CBuffer &buffer)=0;		

	//Will return false if the GAME server doesnt want to accept the client
	virtual bool ValidateClConnection(int clNum, 
									  bool reconnect,
									  CBuffer &buffer)=0; 
	//Handle client spawn
	virtual void OnClientSpawn(int clNum)=0;

	//Handle map change for this client ?
	virtual void OnLevelChange(int clNum)=0;

	//Handle client disconnection
	virtual void OnClientDrop (int clNum, EDisconnectReason reason)=0;

	//Have the game server write status info so the network server
	//can respond to a status request
	virtual void WriteGameStatus(CBuffer &buffer)=0;

	//Add more as needed
};

//Predeclarations
namespace VoidNet 
{	
	class CNetSocket;
	class CNetChan;
	class CNetClChan;
}

/*
============================================================================
The Main Server class should create this to handle all network processing
The Network Server is derived from a NetChanWriter class. This class doesnt
have any dependencies, and can be passed around to any other subsystems
including any game server dlls
============================================================================
*/
class CNetServer : public NetChanWriter
{
public:

	//Call these on Application Startup/Shutdown
	static bool InitWinsock();
	static void ShutdownWinsock();

	CNetServer();
	~CNetServer();

	//Create the server. Call this first thing
	void Create(I_Server * server, const ServerState * state);

	//Management
	bool Init();
	void Shutdown();
	void Restart();

	//ReadPacket should be called at the beginning of the
	//server frame, and SendPackets at the end
	void ReadPackets();
	void SendPackets();

	//NetChanWriter Implementation
	void ChanBeginWrite(int chanId, byte msgid, int estSize);
	void ChanWrite(byte b);
	void ChanWrite(char c);
	void ChanWrite(short s);
	void ChanWrite(int i);
	void ChanWrite(float f);
	void ChanWrite(const char *string);
	void ChanWriteCoord(float c);
	void ChanWriteAngle(float a);
	void ChanWriteData(byte * data, int len);
	void ChanFinishWrite();

	const NetChanState & ChanGetState(int chanId) const;
	
	//Set the Rate of the given channel
	void ChanSetRate(int chanId, int rate);

	//Reject client connection for given reason
	void SendRejectMsg(const char * reason);
	
	//Have the given Client reconnect
	void SendReconnect(int chanId);

	//Have the client disconnect. Give reason
	void SendDisconnect(int chanId, EDisconnectReason reason);
	
	//Print a Server message to the given client(s)
	void ClientPrintf(int chanId, const char * message, ...);
	void BroadcastPrintf(const char* message, ...);

	//Access functions
	NetSignOnBufs & GetSignOnBufs() { return m_signOnBufs; }
	const char * GetLocalAddr() const { return m_szLocalAddr; }

private:

	//Handle Connectionless requests
	void HandleStatusReq(bool full);
	void HandleConnectReq();
	void HandleChallengeReq();

	//Parse OOB query
	void ProcessQueryPacket();				
		
	//Client hasn't spawned yet
	void ParseSpawnMessage(int chanId);		
	void SendSpawnParms(int chanId);

	//===================================================

	struct NetChallenge;
	NetChallenge  * m_challenges;

	I_Server      * m_pServer;		//Main Server

	NetSignOnBufs	m_signOnBufs;

	VoidNet::CNetSocket * m_pSock;	//The Socket
	VoidNet::CNetClChan * m_clChan;	//Client channels

	const ServerState * m_pSvState;	//Server Status

	char	m_szLocalAddr[MAX_IPADDR_LEN];
	
	CBuffer	m_recvBuf;
	CBuffer	m_sendBuf;

	char	m_printBuffer[512];
	
	int		m_curChanId;
};

#endif