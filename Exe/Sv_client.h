#ifndef VOID_SV_CLIENT
#define VOID_SV_CLIENT

#include "Net_chan.h"

/*
======================================
Server side client data
======================================
*/
struct SVClient
{
	enum
	{	MAX_BACKBUFFERS = 4
	};
	
	SVClient();
	~SVClient();

	void MakeSpace(int maxsize);
	void ValidateBuffer();
	void Reset();
	
	//fix me, shoud be cahr
	void BeginMessage(byte msgid, int estSize);
	
	void WriteByte(byte b);
	void WriteChar(char c);
	void WriteShort(short s);
	void WriteInt(int i);
	void WriteFloat(float f);
	void WriteCoord(float c);
	void WriteAngle(float a);
	void WriteString(const char *string);
	void WriteData(byte * data, int len);

	bool ReadyToSend();

	//Flags and States
	bool		m_bDropClient;	//drop client if this is true
	bool		m_bSend;

	//keep track of how many spawn messages have been sent
	//when it equals SVC_BEGIN, then the client will be assumed to have spawned
	byte		m_spawnState;

	int			m_state;

//Private?
	CNetChan 	m_netChan;

	//back buffers for client reliable data
	bool		m_bBackbuf;
	int			m_numBuf;
//	CBuffer* m_backBuffer[MAX_BACKBUFFERS];
	CBuffer		m_backBuffer[MAX_BACKBUFFERS];

	//Game specifc
	int			m_id;
	char		m_name[32];
};

#endif