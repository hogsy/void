#if 0

//This is what the server runs
#define MAX_CLASSNAME 32

class CEntBase
{
public:
	char classname[MAX_CLASSNAME];

	unsigned int sector_index;		// which sector the entitiy is in
//	sector_t *sector;

	vector_t origin;
	vector_t angles;

//	virtual void Think();			//every server frame
};

class CSVClient:public CEntBase
{
public:

	CSVClient();
	~CSVClient();

	bool SV_InitClient(SOCKADDR_IN laddr, int port);				//init the client socket and bind to this info
	
	bool SV_Connect(SOCKADDR_IN raddr, int rport, int numclient);	//connect to this info
	bool SV_Disconnect();											//disconnect cleanup the client

	void WriteToClient(int type,unsigned char *msg);				//append this to the clients outgoing buffer

	void Run();

	//Client states
	bool		m_active;
	bool		m_ingame;
	bool		m_connected;

	
	//Net info vars
	int			m_port;				//this will be the clients port
	float		m_lastmessagetime;	//reliable messages must be sent
	int			m_retries;			//message retries
	float		m_rate;
	

	CNBuffer	 m_recvBuf;			//message out, added to outgoing socket buffer
	CNBuffer	 m_sendBuf;			//this gets updated with data coming in the socket

	CSocket	 	 m_sock;
	
	unsigned int m_sendseq;
	unsigned int m_recvseq;

	
	//Game specific
	//these could be moved to a seperate struct or class
	//which could be shared by the game dlls
	
	int			num;
	char		name[32];
	char		ipaddr[16];

	eyepoint_t  eye;
	vector_t	desired_movement;
};

#endif



