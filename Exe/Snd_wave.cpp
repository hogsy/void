#include  "Snd_wave.h"

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
		ulong  averageBytesPerSec;	// generally samplesPerSecond*blockAlign
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

using namespace VoidSound;

CFileBuffer CWaveFile::m_fileReader;

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

	m_filename = 0;
}

CWaveFile::CWaveFile(const char * wavefile)
{
	m_samplesPerSecond = m_size = 0;
	m_blockAlign = 0;
	m_data = 0;
	m_filename = 0;
	LoadFile(wavefile);
}

/*
==========================================
Destructor
==========================================
*/


CWaveFile::~CWaveFile()
{
	Unload();
}
	
/*
==========================================
Load a given file
==========================================
*/

bool CWaveFile::LoadFile(const char * wavefile)
{
	if(!m_fileReader.Open(wavefile))
	{
		ComPrintf("CWaveFile::LoadFile: Failed to load %s\n", wavefile);
		return false;
	}

	wavFileHeader wFilehdr;
	m_fileReader.Read(&wFilehdr,sizeof(wavFileHeader),1);
	
	//make sure its a RIFF or a WAVE file
	if ((wFilehdr.marker != ('R' + ('I'<<8) + ('F'<<16) + ('F'<<24))) ||
	    (wFilehdr.waveMarker != ('W' + ('A'<<8) + ('V'<<16) + ('E'<<24))))
	{
		m_fileReader.Close();
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

		m_fileReader.Seek(chunkPos,SEEK_SET);
		m_fileReader.Read(&wChunkHdr,chunkSize,1);

		//Found the Format Chunk
		if (wChunkHdr.marker == ('f' + ('m'<<8) + ('t'<<16) + (' '<<24)))
		{
			//Go back and read it into the format data
			m_fileReader.Read(&wFormatChunk,sizeof(wavFormatChunk),1);

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
			m_fileReader.Read(m_data,m_size,1);

			//Break if we have read both
			if(wFormatChunk.formatTag && m_data)
			{
				readWave = true;
				break;
			}
		}
	}
	m_fileReader.Close();

	if(!readWave)
	{
		ComPrintf("CWaveFile::LoadFile:Error reading %s\n",wavefile);
		return false;
	}

	m_filename = new char[(strlen(wavefile)+1)];
	strcpy(m_filename,wavefile);
	ComPrintf("Read %s, size %d, block %d, samples per sec %d\n", m_filename,m_size, m_blockAlign, m_samplesPerSecond);
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
		m_size = 0;
	}
}
	
bool CWaveFile::IsEmpty() const
{	
	if(!m_data)	
		return true; 
	return false;
}
