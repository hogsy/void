#ifndef  VOID_NETWORK_SERVER
#define  VOID_NETWORK_SERVER

#include "Com_buffer.h"
#include "Net_defs.h"

//Predeclarations
namespace VoidNet 
{	
	class CNetSocket;
	class CNetChan;
	class CNetClChan;
}

/*
======================================
callback interface which should be 
implemented by the main server class
======================================
*/
struct I_Server
{
	//process ingame client message
	virtual void HandleClientMsg(int chanId, CBuffer &buffer)=0;		

	//Will return false, and reject reason in FAILED if rejected
	virtual bool ValidateClConnection(int chanId, bool reconnect,
							  CBuffer &buffer)=0; 

	virtual void OnClientSpawn(int chanId)=0;
	virtual void OnLevelChange(int chanId)=0;

	//reason is given if it was network related
	virtual void OnClientDrop(int chanId, int state,
							  const char * reason=0)=0;
};


/*
======================================
The Network Server
======================================
*/
class CNetServer : public NetChanWriter
{
public:

	CNetServer();
	~CNetServer();

	void Create(I_Server * server, ServerState * state);

	bool Init();
	void Shutdown();
	void Restart();

	void ReadPackets();
	void SendPackets();

	void ClientPrintf(int chanId, const char * message, ...);
	void BroadcastPrintf(const char* message, ...);
	
	//To current connecting client	
	void SendRejectMsg(const char * reason);

	void SendReconnect(int chanId);
	void SendDisconnect(int chanId, const char * reason);

	//NetChanWriter Implementation
	void BeginWrite(int chanId, byte msgid, int estSize);
	void Write(byte b);
	void Write(char c);
	void Write(short s);
	void Write(int i);
	void Write(float f);
	void Write(const char *string);
	void WriteCoord(float c);
	void WriteAngle(float a);
	void WriteData(byte * data, int len);
	void FinishWrite();

	const NetChanState & GetState(int chanId) const;
	
	//Set the Rate of the given channel
	void SetChanRate(int chanId, int rate);

	//Call these on Application Startup/Shutdown
	static bool InitWinsock();
	static void ShutdownWinsock();

//FIXME
	enum
	{
		MAX_SIGNONBUFFERS = 8,
		MAX_CHALLENGES =  512
	};
	//Used to stores Entity baselines etc which
	//are transmitted to the client on connection
	int			m_numSignOnBuffers;
	CBuffer		m_signOnBuf[MAX_SIGNONBUFFERS];

private:

	//Handle Connectionless requests
	void HandleStatusReq();
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

	VoidNet::CNetSocket * m_pSock;	//The Socket
	VoidNet::CNetClChan * m_clChan;	//Client channels

	ServerState * m_pSvState;	//Server Status
	I_Server    * m_pServer;	//Main Server

	CBuffer	m_recvBuf;
	CBuffer	m_sendBuf;

	char	m_printBuffer[512];
	int		m_curChanId;
};

#endif