#ifndef VOID_SND_WAVEFILE
#define VOID_SND_WAVEFILE

#include "Com_res.h"

namespace VoidSound {

/*
==========================================
Util class which reads in Wave file data
We do NOT support Stereo wave files
==========================================
*/
class CWaveFile
{
public:

	const byte * GetData() const	{ return m_data; }
	const char * GetFileName() const{ return m_filename; }
	
	//Directly accessible Stats
	ulong  m_size;
	ulong  m_samplesPerSecond;
	ushort m_blockAlign;
	ushort m_bitsPerSample;

private:

	friend class CResManager<CWaveFile>;

	CWaveFile();
	~CWaveFile();
	
	bool Load(const char * szFileName, I_FileReader * pFile);
	void Unload();
	bool IsEmpty() const;

	int	   m_refs;		//Refcounts
	char * m_filename;	//Resource manager will use this filename for searching.

	byte * m_data;		// data stream, variable-sized
};

typedef CResManager<CWaveFile> CWaveManager;

}

#endif