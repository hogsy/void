#ifndef _NET_UTIL
#define _NET_UTIL

#include "Sys_hdr.h"
#include "Net_defs.h"

extern	char	g_computerName[256];
extern	char	g_ipaddr[16];

bool ValidIP(char *ip);
void PrintSockError(int err=0);

bool InitWinsock();


/*
=====================================
Network buffer class
this is sort of a hack and is only exposed to the CSocket class
so that it can update buffers sizes and reset data after sending etc
=====================================
*/

class CBaseNBuffer
{
public:
	CBaseNBuffer(int size=MAX_DATAGRAM);
	~CBaseNBuffer();

	virtual void Reset();
	
	int				cursize;		//current size, updated whenever the buffer is updated
	BYTE		*	data;			//data
protected:
	int				maxsize;		//max size for the buffer
};


/*
=====================================
this is the derived class that is used by the clients
for writing and reading data as its updated by the socket.
=====================================
*/


class CNBuffer:public CBaseNBuffer
{
public:
	CNBuffer(int size=MAX_DATAGRAM);
	~CNBuffer();


	//Writing funcs
	void WriteChar(int i);
	void WriteByte(int i);
	void WriteShort(int i);
	void WriteLong(int i);
	void WriteFloat(float f);
	void WriteAngle(float f);
	void WriteCoord(float f);
	void WriteString(const char *s);

	//Reading funcs
	int   ReadChar(void);
	int   ReadByte(void);
	int	  ReadShort(void);
	int	  ReadLong(void);
	float ReadFloat(void);
	float ReadAngle(void);
	float ReadCoord(void);
	char* ReadString(void);
	char* ReadString(char delim);

	void  Reset();

private:
	
	int				readcount;		//how much have we read

	//flags
	bool			overflowed;
	bool			allowoverflow;
	bool			badread;

	static char		tstring[2048];

	void  Clear()	{ cursize = 0;}

	void* GetSpace(int	len);
	int	  LongSwap(int i);
	void  Write (void *data, int length);
};

#endif