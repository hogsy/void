#include "Sys_hdr.h"
#include "Snd_wave.h"
#include "Snd_main.h"


/*
======================================
Private Structs
Wave Format
======================================
*/

typedef struct
{
	unsigned long marker;		// four-character chunk type marker
	unsigned long length;		// length of chunk after this header
} wavChunkHeader_t;

typedef struct
{
	wavChunkHeader_t header;	// marker "data"
	unsigned char data[1];		// data stream, variable-sized
} wavDataChunk_t;

typedef struct
{
	wavChunkHeader_t header;				// marker "fmt "
	unsigned short formatTag;				// should be 1
	unsigned short numChannels;				// 1=mono, 2=stereo
	unsigned long samplesPerSecond;			// playback rate
	unsigned long averageBytesPerSecond;	// generally samplesPerSecond*blockAlign
	unsigned short blockAlign;				// block alignment, bytesPerSample = blockAlign / numChannels
	unsigned short formatSpecific;			// reserved I think
} wavFormatChunk_t;

typedef struct
{
	unsigned long marker;		// "RIFF"
	unsigned long waveChunkLen; // size of wave data chunk that follows (i.e. usually filesize = waveChunkLen+8)
	unsigned long waveMarker;	// "WAVE"
	// every chunk that follows is up for grabs, but hunt down the "fmt " and "data" chunks
} wavFileHeader_t;



/*
======================================
Low level - wave loading func
Fills the Wavefile struct with as much info as it can
======================================
*/

bool LoadWaveFile(char *filename, CWavefile *data)
{
	FILE * fp;
	wavFileHeader_t* fhdr;
	wavChunkHeader_t* chdr;
	wavFormatChunk_t* format;
	wavDataChunk_t  *wavedata;
	unsigned long chunkPos;
	unsigned long size;

	char temp[256];
	strcpy(temp,CSound::soundpath);
	strcat(temp,filename);

	//open it in binary
	if(!(fp = fopen(temp,"rb")))
	{
		ComPrintf("::LoadWaveFile - Couldnt open file - %s\n",temp);
		return false;
	}

	//Get size of file
	if(fseek(fp, 0, SEEK_END))		//go to end
	{
		ComPrintf("::LoadWaveFile - Couldnt read 1 file - %s\n",temp);
		return false;
	}
	size= ftell(fp);				//get position
	if(fseek(fp,0,SEEK_SET))		//go back to start
	{
		ComPrintf("::LoadWaveFile - Couldnt read 2 file - %s\n",temp);
		return false;
	}

	data->fpBuf = (unsigned char*)malloc(size);	//alloc buffer
	
	if (fread(data->fpBuf, 1, size, fp) != size)
	{
		ComPrintf("::LoadWaveFile - couldnt reed entire file - %s\n",temp);
		return false;
	}
	fclose(fp);						//done reading into buffer
	
	//make sure its a RIFF or a WAVE file
	fhdr = (wavFileHeader_t*)data->fpBuf;
	if ((fhdr->marker != ('R' + ('I'<<8) + ('F'<<16) + ('F'<<24)))
	 || (fhdr->waveMarker != ('W' + ('A'<<8) + ('V'<<16) + ('E'<<24))))
	{
		free(data->fpBuf);
		ComPrintf("::LoadWaveFile - invalid wave format - %s",temp);
		return false;
	}
	
	wavedata = NULL;
	format = NULL;

	for (chunkPos=sizeof(wavFileHeader_t);
	     chunkPos<(fhdr->waveChunkLen+sizeof(wavChunkHeader_t));
		 chunkPos+=(chdr->length+sizeof(wavChunkHeader_t)) )
	{
		if ((wavedata) && (format))
			break;
		
		chdr = (wavChunkHeader_t*)(data->fpBuf+chunkPos);
		
		if (chdr->marker == ('f' + ('m'<<8) + ('t'<<16) + (' '<<24)))
			format = (wavFormatChunk_t*)chdr;
		
		else if (chdr->marker == ('d' + ('a'<<8) + ('t'<<16) + ('a'<<24)))
			wavedata = (wavDataChunk_t*)chdr;
			
	}
	if ((!wavedata) || (!format))
	{
		free(data->fpBuf);
		ComPrintf("::LoadWaveFile - couldnt read wave data - %s",temp);
		return false;
	}

	data->length = wavedata->header.length;
	data->samplesPerSecond = format->samplesPerSecond;
	data->numChannels = format->numChannels;
	data->blockAlign = format->blockAlign;	
	data->data = wavedata->data;
	data->filename = new char[(strlen(filename)+1)];
	strcpy(data->filename,filename);
	return true;
}


/*
======================================
Unloading func - not needed
======================================
*/

bool UnloadWaveFile(int index)
{
	return true;
}


//============================================================================

/*
======================================
Wavemanager constructor
======================================
*/

CWavemanager::CWavemanager()
{
	wavepool[0].filename =0;
	wavepool[0].data = 0;
	wavepool[0].fpBuf =0;
	curwaves = 1;
}

/*
======================================
Wavemanager destructor
======================================
*/

CWavemanager::~CWavemanager()
{
}

/*
======================================
Register a Wave file
======================================
*/

int	CWavemanager::Register(char *filename,int cache_type)
{
	//int index =curwaves;
	//Check if we already have this file

	//Alloc space for file
	if(LoadWaveFile(filename, &wavepool[curwaves]))
	{
		ComPrintf("CWavemanger::Registered Wave:%s at %d\n",filename,curwaves);
		curwaves++;
		return (curwaves-1);
	}
	return 0;

}


/*
======================================
Unregister
======================================
*/
bool CWavemanager::Unregister(int index)
{
	return true;
}


/*
======================================
Get wave file from index
======================================
*/
CWavefile * CWavemanager::WaveIndex(int index)
{
	if(!index || index >= curwaves)
		return 0;

	if(wavepool[index])
		return &wavepool[index];
	return 0;
}


/*
======================================
Get wave file from Filename
======================================
*/
CWavefile * CWavemanager::WaveIndex(char *filename)
{
	for(int i=1;i<curwaves;i++)
	{
		if(!strcmp(wavepool[i].filename,filename))
			return &wavepool[i];
	}
	return 0;
}

/*
======================================
Get index from filename
======================================
*/

int CWavemanager::GetIndex(char *filename)
{
	for(int i=1;i<curwaves;i++)
	{
		if(!strcmp(wavepool[i].filename,filename))
			return i;
	}
	return 0;
}











