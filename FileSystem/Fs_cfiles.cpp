




//====================================================================================
//C interface to file streams
/*
extern "C"
{
	FILESYSTEM_API uint FS_Open(const char *name);
	FILESYSTEM_API void FS_Close(uint handle);
	FILESYSTEM_API int  FS_Read(void *buffer,int size,uint handle);
	FILESYSTEM_API void FS_Seek(uint handle,int offset, signed char mode);
	FILESYSTEM_API int  FS_Tell(uint handle);
}

*/



#if 0


#include "Fs_hdr.h"
#include "I_file.h"

//======================================================================================
//C Interface for FileSteam I/O
//======================================================================================

static const int	MAX_FILESTREAMS = 16;

static CFileStream	m_fileStreams[MAX_FILESTREAMS+1];
static int			m_numOpenFiles   = 0;


//======================================================================================
//======================================================================================

//Returns 0 if failed.
FILESYSTEM_API uint FS_Open(const char *name)
{
ComPrintf("opening %s, %d files open\n", name, m_numOpenFiles);

	if(m_numOpenFiles >= MAX_FILESTREAMS)
		return 0;
	
	//find empty stream, start with item 2, because we can't return 0
	for(int i=1;i<MAX_FILESTREAMS;i++)
	{
		if(m_fileStreams[i].isOpen() == false)
			break;
	}
	//didnt find any
	if(i== MAX_FILESTREAMS)
		return 0;

	if(m_fileStreams[i].Open(name))
	{
		m_numOpenFiles ++;
ComPrintf("opened %s\n", name);
		return i;
	}
	return 0;
}


FILESYSTEM_API void FS_Close(uint handle)
{
	if(m_fileStreams[handle].isOpen())
	{
ComPrintf("closed %s\n", m_fileStreams[handle].GetFileName());
		m_numOpenFiles --;
		m_fileStreams[handle].Close();
	}
}


FILESYSTEM_API int  FS_Read(void *buffer,int size,uint handle)
{
	if(m_fileStreams[handle].isOpen())
	{
		int read = m_fileStreams[handle].Read(buffer,1,size);
ComPrintf("reading %s, %d bytes, %d read\n", m_fileStreams[handle].GetFileName(), size, read);
	}
	return 0;
}


FILESYSTEM_API void FS_Seek(uint handle,int offset, signed char mode)
{
	if(m_fileStreams[handle].isOpen())
	{
		int rmode = (int)mode;
		if(mode == 0)
ComPrintf("seeking %s, to %d SEEK_SET\n", m_fileStreams[handle].GetFileName(),offset);
		else if(mode == 1)
ComPrintf("seeking %s, to %d SEEK_CUR\n", m_fileStreams[handle].GetFileName(),offset);
		else if(mode == 2)
ComPrintf("seeking %s, to %d SEEK_END\n", m_fileStreams[handle].GetFileName(),offset);

		m_fileStreams[handle].Seek(offset,rmode);
	}
}

FILESYSTEM_API int  FS_Tell(uint handle)
{
	if(m_fileStreams[handle].isOpen())
	{
		int pos = (int)m_fileStreams[handle].GetPos();
ComPrintf("telling %s , at %d\n", m_fileStreams[handle].GetFileName(), pos);
		return pos;
	}
	return 0;
}


#endif