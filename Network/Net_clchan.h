#ifndef VOID_NET_CLIENTCHAN
#define VOID_NET_CLIENTCHAN

#include "Net_chan.h"

namespace VoidNet {

/*
======================================
The "Game" Server needs to be able
to write directly to client channels.
======================================
*/
class CNetClChan
{
public:
	enum
	{	MAX_BACKBUFFERS = 4
	};

	CNetClChan();
	~CNetClChan();

	void MakeSpace(int maxsize);
	void ValidateBuffer();
	void Reset();
	bool ReadyToSend();

	CNetChan m_netChan;

	//Flags and States
	bool	m_bDropClient;	//drop client if this is true
	bool	m_bSend;		//send this frame if this is true

	//keep track of how many spawn messages have been sent
	//when it equals SVC_BEGIN, then the client will be assumed to have spawned
	byte	m_spawnState;	
	int     m_spawnLevel;

	int		m_state;

	//back buffers for client reliable data
	bool	m_bBackbuf;
	int		m_numBuf;
	CBuffer	m_backBuffer[MAX_BACKBUFFERS];
};

}
#endif