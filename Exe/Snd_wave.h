#ifndef VOID_SND_WAVEFILE
#define VOID_SND_WAVEFILE

//forward declare file
class CFileBuffer;

namespace VoidSound {

/*
==========================================
Util class which reads in Wave file data
We do NOT support Stereo wave files
==========================================
*/

class CWaveFile
{
	friend class CWaveManager;

	CWaveFile();
	~CWaveFile();
	
	bool LoadFile(const char * wavefile, CFileBuffer * pFile);
	void Unload();
	
	bool IsEmpty() const;

	int	   m_refs;
	char * m_filename;
	byte * m_data;		// data stream, variable-sized

public:

	const byte * GetData() const	{ return m_data; }
	const char * GetFileName() const{ return m_filename; }
	
	//Directly accessible Stats
	ulong  m_size;
	ulong  m_samplesPerSecond;
	ushort m_blockAlign;
	ushort m_bitsPerSample;

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

	CWaveManager(int maxResources);
	~CWaveManager();

	//Get a waveFile from the manager. it will create if its new
	//and will increment counter and return a copy if it exists
	CWaveFile * Create(const char * szFileName);

	//Release a wave resource. will be unloaded if no one is using it now
	int Release(CWaveFile * wave);

private:

	struct WaveList
	{
		WaveList() : waveFile(0), next(0) {}
		~WaveList() { waveFile = 0; next = 0; }

		CWaveFile * waveFile;
		WaveList  * next;
	};

	int			   m_maxItems;
	CWaveFile   *  m_waveCache;

	WaveList	*  m_freeWaves;
	WaveList	*  m_usedWaves;

	CFileBuffer *  m_pFileReader;
};


}

#endif