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
	CL_INUSE = 1,		//socket is active, trying to connect or something
	CL_CONNECTED = 2,	//connected, need to get prespawn info
	CL_INGAME = 4		//in game
};

/*
======================================
Other misc shared definitions
======================================
*/
const int  NET_TIMEOUT_INTERVAL= 10;	//10 seconds
const int  NET_PACKET_HEADER   = 8;
const int  NET_IPADDRLEN	   = 24;
const int  NET_MAXDATAGRAMSIZE = 1450;
const int  NET_MAXBUFFERSIZE   = 2900;
const int  NET_SVDEFAULTPORT   = 20010;
const int  NET_CLDEFAULTPORT   = 20011;
const int  NET_MAXCLIENTS	   = 32;

//This needs to be defined for the library to link
namespace System {
	const float &	GetCurTime();
}

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
		latency = 0.0f;
	}
	
	uint	inMsgId;			//Latest incoming messageId
	uint	inAckedId;			//Latest remotely acked message.
	uint	outMsgId;			//Outgoing messageId
	uint	lastOutReliableId;	//Id of the last reliable message sent
	int		dropCount;			//Number of packets that went missing
	int		goodCount;			//Number of packed received okay
	int		numChokes;			//Times we throttled back to conform to the rate
	float   latency;
};

/*
======================================
Simple Multicast struct
Can use this to have the Network server
write a to bunch of clients
======================================
*/
struct MultiCastSet
{
	MultiCastSet() 
	{	Reset();
	}
	
	//every one but the given client
	explicit MultiCastSet(int clNum) 
	{	memset(&dest,true, sizeof(bool) * NET_MAXCLIENTS);
		dest[clNum] = false;
	}

	MultiCastSet(const MultiCastSet &set)
	{	for(int i=0; i<NET_MAXCLIENTS; i++)
			dest[i] = set.dest[i];
	}

	MultiCastSet & operator = (const MultiCastSet &set)
	{	for(int i=0; i<NET_MAXCLIENTS; i++)
			dest[i] = set.dest[i];
		return *this;
	}

	void Reset() 
	{ memset(&dest,false, sizeof(bool) * NET_MAXCLIENTS); 
	}
	
	bool dest[NET_MAXCLIENTS];
};

/*
======================================
Network channel writer
chanID correponds to clientNum
======================================
*/
struct NetChanWriter
{
	virtual bool ChanCanSend(int chanId)=0;
	
	virtual void ChanBeginWrite(int chanId, byte msgid, int estSize)=0;
	virtual void ChanBeginWrite(const MultiCastSet &set, 
								byte msgid, int estSize)=0;

	virtual void ChanWriteByte(byte b)=0;
	virtual void ChanWriteChar(char c)=0;
	virtual void ChanWriteShort(short s)=0;
	virtual void ChanWriteInt(int i)=0;
	virtual void ChanWriteFloat(float f)=0;
	virtual void ChanWriteString(const char *string)=0;
	virtual void ChanWriteCoord(float c)=0;
	virtual void ChanWriteAngle(float a)=0;
	virtual void ChanWriteData(byte * data, int len)=0;
	virtual void ChanFinishWrite()=0;
	
	virtual const NetChanState & ChanGetState(int chanId) const=0;
};

#endif