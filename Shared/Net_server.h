#ifndef  VOID_NETWORK_SERVER
#define  VOID_NETWORK_SERVER

#include "Com_buffer.h"
#include "Net_defs.h"

/*
============================================================================
The Game server usually needs to know how to handle a client disconnection
For example the game server shouldnt broadcast a Client disconnection
if it was the Server which went down.
The OnClientDrop function includes a DisconnectReason parameter.
============================================================================
*/
struct DisconnectReason
{	
	const char * disconnectMsg;	//message sent to client as reason
	const char * broadcastMsg;	//message broadcast to connected clients
};

const DisconnectReason DR_SVQUIT	= {"Server quit",0};
const DisconnectReason DR_SVERROR	= {"Server error", 0};
const DisconnectReason DR_CLQUIT	= {"Disconnected","%s disconnected" };
const DisconnectReason DR_CLTIMEOUT = {"Connection timed out","%s timed out" };
const DisconnectReason DR_CLOVERFLOW= {"Connection overflowed","%s overflowed" };
const DisconnectReason DR_CLBADMSG	= {"Network message error","%s errored out" };
//Add More as needed
const DisconnectReason DR_SVKICKED  = {"You were kicked","%s was kicked" };

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
	//Put client into the map
	virtual void OnClientBegin(int clNum)=0;

	//Handle map change for this client ?
	virtual void OnLevelChange(int clNum)=0;

	//Handle client disconnection
	virtual void OnClientDrop (int clNum, const DisconnectReason &reason)=0;

	//Have the game server write status info so the network server
	//can respond to a status request
	virtual void WriteGameStatus(CBuffer &buffer)=0;

	//Return number of buffers for the given configString
	virtual int  NumConfigStringBufs(int stringId) const = 0;

	//Have the game server write a configString to the given buffer
	//shouldnt be more than the max size, return false if invalid parms are given
	virtual bool WriteConfigString(CBuffer &buffer, int stringId, int numBuffer=0)=0;

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

	//Management
	bool Init(I_Server * server, const ServerState * state);
	void Shutdown();
	void Restart();

	//ReadPacket should be called at the beginning of the
	//server frame, and SendPackets at the end
	void ReadPackets();
	void SendPackets();

	//NetChanWriter Implementation
	bool ChanCanSend(int chanId);
	void ChanBeginWrite(int chanId, byte msgid, int estSize);
	void ChanBeginWrite(const MultiCastSet &set, byte msgid, int estSize);
	void ChanWriteByte(byte b);
	void ChanWriteChar(char c);
	void ChanWriteShort(short s);
	void ChanWriteInt(int i);
	void ChanWriteFloat(float f);
	void ChanWriteString(const char *string);
	void ChanWriteCoord(float c);
	void ChanWriteAngle(float a);
	void ChanWriteData(byte * data, int len);
	void ChanFinishWrite();

	const NetChanState & ChanGetState(int chanId) const;
	
	//Set the Rate of the given channel
	void ChanSetRate(int chanId, int rate);
	int  ChanGetRate(int chanId);

	//Reject client connection for given reason
	void SendRejectMsg(const char * reason);
	
	//Have the given Client reconnect
	void SendReconnect(int chanId);

	//Have the client disconnect. Give reason
	void SendDisconnect(int chanId, const DisconnectReason &reason);
	
	//Print a Server message to the given client(s)
	void ClientPrintf(int chanId, const char * message);
	void BroadcastPrintf(const char* message);

	//Access functions
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
	CBuffer	m_recvBuf;
	CBuffer	m_sendBuf;

	char	m_szLocalAddr[NET_IPADDRLEN];

	struct NetChallenge;
	NetChallenge		* m_challenges;
	I_Server			* m_pServer;	//Main Server
	const ServerState	* m_pSvState;	//Server Status

	VoidNet::CNetSocket * m_pSock;		//The Socket
	VoidNet::CNetClChan * m_clChan;		//Client channels

	//CNetChan destination data
	int				m_curChanId;
	MultiCastSet	m_multicastInfo;
};

#endif