#include "Sys_hdr.h"
#include "I_file.h"
#include "Snd_wave.h"

using namespace VoidSound;

/*
================================================
Constructor
================================================
*/
CWaveManager::CWaveManager(int maxResources) : m_maxItems(maxResources)
{	
	m_waveCache = new CWaveFile [m_maxItems];
	m_pFileReader = new CFileBuffer();


	m_freeWaves;
	m_usedWaves;
}


/*
================================================
Destructor
================================================
*/
CWaveManager::~CWaveManager()
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

/*
================================================
Get a waveFile from the manager. it will create if its new
and will increment counter and return a copy if it exists
TODO, could use a hashing scheme here.
================================================
*/
CWaveFile * CWaveManager::Create(const char * szFileName)
{
	//first check for duplicates in files which are in use
	int freeIndex = -1;
	for(int i=0; i<m_maxItems; i++)
	{
		if(m_waveCache[i].IsEmpty())
		{
			if(freeIndex == -1)
				freeIndex = i;
		}
		else if(_stricmp(m_waveCache[i].m_filename, szFileName) == 0)
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
	if(m_waveCache[freeIndex].LoadFile(szFileName, m_pFileReader))
	{
		m_waveCache[freeIndex].m_refs ++;
		return &m_waveCache[freeIndex];
	}
	return 0;
}

/*
================================================
Release a wave resource. will be 
unloaded if no one is using it now
================================================
*/
int CWaveManager::Release(CWaveFile * wave)
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




//======================================================================================
//======================================================================================


namespace
{
	struct wavChunkHeader
	{
		wavChunkHeader() { marker = length = 0; }

		ulong marker;		// four-character chunk type marker
		ulong length;		// length of chunk after this header
	};

	struct wavFormatChunk
	{
		wavFormatChunk() 
		{ 
			formatTag = numChannels = 0;
			samplesPerSec = averageBytesPerSec = 0;
			blockAlign = bitsPerSample = 0;
		}
		ushort formatTag;			// should be 1
		ushort numChannels;			// 1=mono, 2=stereo
		ulong  samplesPerSec;		// playback rate
		ulong  averageBytesPerSec;	// samplesPerSecond*blockAlign
		ushort blockAlign;			// block alignment, bytesPerSample = blockAlign / numChannels
		ushort bitsPerSample;
	};

	struct wavFileHeader
	{
		wavFileHeader() { marker = waveChunkLen = waveMarker = 0; }

		ulong marker;		//"RIFF"
		ulong waveChunkLen; //size of wave data chunk that follows (i.e. usually filesize = waveChunkLen+8)
		ulong waveMarker;	//"WAVE"
		// every chunk that follows is up for grabs, but hunt down the "fmt " and "data" chunks
	};
}

//======================================================================================
//======================================================================================

/*
==========================================
Constructor
==========================================
*/

CWaveFile::CWaveFile()
{
	m_samplesPerSecond = m_size = 0;
	m_blockAlign = 0;
	m_data = 0;
	m_refs = 0;
	m_filename = 0;
}

/*
==========================================
Destructor
==========================================
*/
CWaveFile::~CWaveFile()
{	Unload();
}
	
/*
==========================================
Load a given file
==========================================
*/

bool CWaveFile::LoadFile(const char * wavefile, CFileBuffer * pFile)
{
	if(!pFile->Open(wavefile))
	{
		ComPrintf("CWaveFile::LoadFile: Failed to load %s\n", wavefile);
		return false;
	}

	wavFileHeader wFilehdr;
	pFile->Read(&wFilehdr,sizeof(wavFileHeader),1);
	
	//make sure its a RIFF or a WAVE file
	if ((wFilehdr.marker != ('R' + ('I'<<8) + ('F'<<16) + ('F'<<24))) ||
	    (wFilehdr.waveMarker != ('W' + ('A'<<8) + ('V'<<16) + ('E'<<24))))
	{
		pFile->Close();
		ComPrintf("CWaveFile::LoadFile: Invalid wave format, %s",wavefile);
		return false;
	}


	//First chunk starts after the fileheader
	ulong chunkSize = sizeof(wavChunkHeader);
	ulong chunkPos = sizeof(wavFileHeader);

	bool readWave = false;

	//Read info into these strucys
	wavChunkHeader wChunkHdr;
	wavFormatChunk wFormatChunk;
	
	//Loop through chunk headers until we find info on wavedata and format type
	for(chunkPos; chunkPos < (wFilehdr.waveChunkLen + chunkSize); chunkPos += (wChunkHdr.length + chunkSize))
	{

		pFile->Seek(chunkPos,SEEK_SET);
		pFile->Read(&wChunkHdr,chunkSize,1);

		//Found the Format Chunk
		if (wChunkHdr.marker == ('f' + ('m'<<8) + ('t'<<16) + (' '<<24)))
		{
			//Go back and read it into the format data
			pFile->Read(&wFormatChunk,sizeof(wavFormatChunk),1);

			m_samplesPerSecond = wFormatChunk.samplesPerSec;
			//m_numChannels = wFormatChunk.numChannels;
			m_blockAlign = wFormatChunk.blockAlign;
			m_bitsPerSample = wFormatChunk.bitsPerSample;
		}
		//Found the Data Chunk
		else if (wChunkHdr.marker == ('d' + ('a'<<8) + ('t'<<16) + ('a'<<24)))
		{
			m_size = wChunkHdr.length;
			m_data = (byte*)g_pHunkManager->HunkAlloc(m_size);  //new byte[m_length];
			pFile->Read(m_data,m_size,1);

			//Break if we have read both
			if(wFormatChunk.formatTag && m_data)
			{
				readWave = true;
				break;
			}
		}
	}
	pFile->Close();

	if(!readWave)
	{
		ComPrintf("CWaveFile::LoadFile:Error reading %s\n",wavefile);
		return false;
	}

	m_filename = new char[(strlen(wavefile)+1)];
	strcpy(m_filename,wavefile);
	ComPrintf("Read %s, %d bytes, block %d, samples per sec %d\n", 
		m_filename,m_size, m_blockAlign, m_samplesPerSecond);
	return true;
}


void CWaveFile::Unload()
{
	if(m_filename)
	{
		delete [] m_filename;
		m_filename =0;
	}
	if(m_data)
	{
		g_pHunkManager->HunkFree(m_data);
		m_data = 0;
		
	}
	m_refs = 0;
	m_size = 0;
}
	
bool CWaveFile::IsEmpty() const
{	
	if(!m_data)	
		return true; 
	return false;
}
