#ifndef VOID_SND_WAVEFILE
#define VOID_SND_WAVEFILE

#include "Com_defs.h"
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
private:

	friend class CWaveManager;

	CWaveFile();
	CWaveFile(const char * wavefile);
	~CWaveFile();
	
	bool LoadFile(const char * wavefile);
	void Unload();
	
	bool IsEmpty() const;

	int	   m_refs;
	char * m_filename;
	
	byte * m_data;		// data stream, variable-sized

public:

	const byte * GetData() const	{ return m_data; }
	const char * GetFileName() const{ return m_filename; }
	
	//Stats
	ulong  m_size;
	ulong  m_samplesPerSecond;
	ushort m_blockAlign;
	ushort m_bitsPerSample;

private:

	static CFileBuffer m_fileReader;
};


/*
======================================
The wavecache maintains all currently loaded
wave files, and number of references to them
======================================
*/
class CWaveManager
{
public:

	CWaveManager(int maxResources) : m_maxItems(maxResources)
	{	m_waveCache = new CWaveFile [m_maxItems];
	}

	~CWaveManager()
	{
		//Print diagnostic warnings
		for(int i=0;i<m_maxItems;i++)
		{
			if(!m_waveCache[i].IsEmpty())
			{
				if(m_waveCache[i].m_refs > 0)
					ComPrintf("Warning : %s still has %d references\n", 
							m_waveCache[i].m_filename, m_waveCache[i].m_refs);
				m_waveCache[i].Unload();
			}
		}
		delete [] m_waveCache;
	}

	//Get a waveFile from the manager. it will create if its new
	//and will increment counter and return a copy if it exists
	//TODO, could use a hashing scheme here.
	CWaveFile * Create(const char * szFileName)
	{
		//first check for duplicates
		int freeIndex = -1;
		for(int i=0; i<m_maxItems; i++)
		{
			if(m_waveCache[i].IsEmpty())
			{
				if(freeIndex == -1)
					freeIndex = i;
			}
			else if(strcmp(m_waveCache[i].m_filename, szFileName) == 0)
			{
				m_waveCache[i].m_refs ++;
				return &m_waveCache[i];
			}
		}

		if(freeIndex == -1)
		{
			ComPrintf("CWaveManager::Create: No space to load %s\n", szFileName);
			return 0;
		}

		//Didnt find a matching file, load it now
		if(m_waveCache[freeIndex].LoadFile(szFileName))
		{
			m_waveCache[freeIndex].m_refs ++;
			return &m_waveCache[freeIndex];
		}
		return 0;
	}
	
	//Release a wave resource. will be 
	//unloaded if no one is using it now
	int Release(CWaveFile * wave)
	{
		if(wave)
		{
			wave->m_refs --;
			if(wave->m_refs == 0)
				wave->Unload();
			return wave->m_refs;
		}
		return 0;
	}

private:

	int			  m_maxItems;
	CWaveFile  *  m_waveCache;
};


}

#endif