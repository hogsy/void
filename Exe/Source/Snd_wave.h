#ifndef VOID_SND_WAVEFILE
#define VOID_SND_WAVEFILE

#include "Snd_main.h"
#include "I_file.h"

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
	CWaveFile();
	CWaveFile(const char * wavefile);
	~CWaveFile();
	
	bool LoadFile(const char * wavefile);
	void Unload();
	
	bool IsEmpty() const;

	byte * m_data;		// data stream, variable-sized
	ulong  m_size;
	char * m_filename;

	ulong  m_samplesPerSecond;
	ushort m_blockAlign;
	ushort m_bitsPerSample;

private:

	static CFileBuffer m_fileReader;
};

}

#endif