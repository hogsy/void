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

	virtual bool ValidateClConnection(int chanId, 
							  bool reconnect,
							  CBuffer &buffer, 
							  char ** reason)=0;

	//read clients userInfo vars upon connection
//	virtual void OnClientConnect(int chanId, CBuffer &buffer)=0;

	//client just finished getting prespawn info. spawn it now
	virtual void OnClientSpawn(int chanId)=0;

	virtual void OnLevelChange(int chanId)=0;

	//client disconnected from game
	//reason is given if it was network related
	virtual void OnClientDrop(int chanId, int state,
							  const char * reason=0)=0;

};

/*
struct BroadcastSet
{
	BroadcaseSet(int exempt=0) { 
	bool validDest[SV_MAX_CLIENTS];
};
*/

/*
======================================

======================================
*/
struct ClChanWriter
{
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
};


/*
======================================
The Network Server
======================================
*/
class CNetServer
{
public:

	//Call these on Application Startup/Shutdown
	static bool InitWinsock();
	static void ShutdownWinsock();

	CNetServer(I_Server & server, ServerState & svstate);
	~CNetServer();

	bool Init();
	void Shutdown();
	void Restart();

	void ReadPackets();
	void SendPackets();

	//Write to Network channels
//THIS will be exposed to even the game DLLs
	void ChanBegin(int chanId, byte msgid, int estSize);
	void ChanWrite(int chanId, byte b);
	void ChanWrite(int chanId, char c);
	void ChanWrite(int chanId, short s);
	void ChanWrite(int chanId, int i);
	void ChanWrite(int chanId, float f);
	void ChanWrite(int chanId, const char *string);
	void ChanWriteCoord(int chanId, float c);
	void ChanWriteAngle(int chanId, float a);
	void ChanWriteData(int chanId, byte * data, int len);

	void ChanSetRate(int chanId, int rate);

	const NetChanState & ChanGetState(int chanId) const;

	void ClientPrintf(int chanId, const char * message, ...);
	void BroadcastPrintf(int chanId, const char* message, ...);
	
	void SendReconnect(int chanId);
	void SendDisconnect(int chanId, const char * reason);

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
	//To current connecting client	
	void SendRejectMsg(const char * reason);
	
	//Client hasn't spawned yet
	void ParseSpawnMessage(int chanId);		
	void SendSpawnParms(int chanId);

	//===================================================

	struct NetChallenge;
	NetChallenge  * m_challenges;

	VoidNet::CNetSocket * m_pSock;	//The Socket
	VoidNet::CNetClChan * m_clChan;	//Client channels

	ServerState & m_svState;	//Server Status
	I_Server    & m_server;		//Main Server

	CBuffer		m_recvBuf;
	CBuffer		m_sendBuf;
	
	char		m_printBuffer[512];
};

#endif