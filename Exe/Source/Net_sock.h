#ifndef _V_SOCKET
#define _V_SOCKET

#include <winsock2.h>
#include "Net_defs.h"

/*
=====================================
Socket handling class

encapsulates sending and receiving
is not aware of the game protocol 
and packet construction is consquently 
not done here.
=====================================
*/

class CSocket
{
public:

	enum eSockState
	{
		SOCK_INACTIVE,
		SOCK_IDLE,
		SOCK_CONNECTING,
		SOCK_CONNECTED
	};

	eSockState	m_state;			//socket state

	bool		bcansend;			//can it send, is it connected
	bool		bsend;				//ALWAYS change this to true after filling buffer
	bool		brecv;				//ALWAYS reset this after reading buffer
//	char		m_ipaddr[16];		//the ip address its connected to 

	//time
	double	connecttime;
	double	lastrecvtime;
	double  lastsendtime;


	//unsigned int	sendseq;		//packet num
	//unsigned int	recvseq;		//packet num
	
	CSocket(CBaseNBuffer * recv, CBaseNBuffer *send);	//Constructor
	~CSocket();		
	
	//Initialize the Socket	and pass pointers to buffers					
	bool Init();
	
	bool Bind(SOCKADDR_IN addr, int port, bool loopback=false);
	
	bool Connect(char *ipaddr, int port);			//Connect to this IPADDR
	bool Connect(SOCKADDR_IN raddr, int port);		//Or this Addr info
	bool Disconnect();

	void Run();								//run this socket
	bool Close();							//close it

	bool AcceptConnection();

private:

//	float	timeoutperiod;
	
	int		m_id;

	CBaseNBuffer	*m_recvBuf;
	CBaseNBuffer	*m_sendBuf;

	SOCKET				m_socket;		//the socket
	SOCKADDR_IN			m_addr;			//local addr info, its bound to
	WSAEVENT			m_event;		//Event object for nonblocking more
	LPWSANETWORKEVENTS  m_pNetworkEvents;
	
	static	int m_numSocks;

	bool Recv();		//usually called every frame
	bool Send();		//frequency determined by clients rate setting

	//Error Handling
	void SockError(char *err);
};


#endif