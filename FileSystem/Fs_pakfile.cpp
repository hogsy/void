#include "Fs_pakfile.h"
#include "I_file.h"

//Pak file header
typedef struct
{
	char	id[4];
	int		dirofs;
	int		dirlen;
}PakHeader_t;


/*
===========================================
Constructor/Destructor
===========================================
*/
CPakFile::CPakFile()
{
	m_fp = 0;
	m_files = 0; 
	m_numFiles = 0;

	memset(m_openFiles,0, sizeof(PakOpenFile_t) * CArchive::ARCHIVEMAXOPENFILES);
	m_numOpenFiles = 0;
}

CPakFile::~CPakFile()
{
	if(m_fp)
		fclose(m_fp);

	memset(m_openFiles,0, sizeof(PakOpenFile_t) * CArchive::ARCHIVEMAXOPENFILES);
	m_numOpenFiles = 0;

	if(m_files)
	{
		for(int i=0;i<m_numFiles;i++)
		{
			delete m_files[i];
			m_files[i] = 0;
		}
		delete [] m_files;
		m_files = 0;
	}
}

/*
===========================================
Builds the filelist
===========================================
*/
bool CPakFile::Init(const char * archivepath, const char * basepath)
{
	//Open File
	char filepath[COM_MAXPATH];
	sprintf(filepath,"%s/%s",basepath,archivepath);

	m_fp = fopen(filepath,"r+b");
	if(!m_fp)
	{
		ComPrintf("CPakFile::BuildList: couldnt open %s\n", filepath);
		return false;
	}

	//Read Header
	PakHeader_t phead;
	fread(&phead,sizeof(PakHeader_t),1,m_fp);
	//Verify id
	if (phead.id[0] != 'P' || 
		phead.id[1] != 'A' || 
		phead.id[2] != 'C' || 
		phead.id[3] != 'K')
	{
		ComPrintf("CPakFile::BuildList: %s is not a valid pakfile\n",filepath);
		fclose(m_fp);
		return false;
	}

	//Find num m_files, and alloc space
	m_numFiles = phead.dirlen / PAKENTRYSIZE;

	m_files = new PakEntry_t * [m_numFiles];
	memset(m_files,0, sizeof(PakEntry_t *) * m_numFiles);

	//Seek to the fileinfo offset
	fseek(m_fp,	phead.dirofs, SEEK_SET);

	int i,j, destIndex =0;		
	PakEntry_t * temp= 0;

	for(i= 0; i< m_numFiles;i++)
	{
		temp = new PakEntry_t();
		fread(temp,sizeof(PakEntry_t),1,m_fp);
		
		//Find a place to insert the new entry
		destIndex = i;	//default to last position

		for(j=0;j<i;j++)
		{
			if(!m_files[j])
				break;

			//If new filename is smaller then entry at current index, 
			//then shift all the entires forward to make space for new entry
			if(strcmp(temp->filename, m_files[j]->filename) < 0)
			{
				destIndex = j;
				//start from the last entry and
				//move all the old pointers forward by one
				j = i;
				while(j > destIndex)
				{
					m_files[j] = m_files[j-1];
					j--;
				}
				break;
			}
		}
		m_files[destIndex] = temp;
	}

	strcpy(m_archiveName,archivepath);
	ComPrintf("%s, Added %d entries\n", m_archiveName,m_numFiles);
	return true;
}

/*
==========================================
Check to see if the archive contains this file
==========================================
*/
bool CPakFile::FindFile(char * buf, int buflen,const char * filename)
{
	PakEntry_t * entry=0;
	int filelen = strlen(filename);
	if(BinarySearchForEntry(filename,&entry,0,m_numFiles,filelen))
	{
		char ext[8];
		Util::ParseExtension(ext,8,entry->filename);
		if(strlen(entry->filename) + strlen(m_archiveName) + 2 < buflen)
		{
			sprintf(buf,"%s/%s",m_archiveName,entry->filename,ext);
			return true;
		}
	}
	return false;
}


/*
===========================================
this will have to use a custom memory manager later
on to alloc the buffer for the file.
===========================================
*/
uint CPakFile::LoadFile(byte ** ibuffer, 
						uint buffersize, 
						const char *ifilename)
{
	if(!buffersize && *ibuffer)
	{
		ComPrintf("CPakFile::OpenFile: Expecting empty file pointer %s\n", ifilename);
		return 0;
	}

	PakEntry_t * entry=0;
	if(BinarySearchForEntry(ifilename,&entry,0,m_numFiles))
	{
		if(!buffersize)
		{
			*ibuffer = (byte*)g_pHunkManager->HunkAlloc(entry->filelen);

		}
		else
		{
			if(entry->filelen > buffersize)
			{
				ComPrintf("CPakFile::LoadFile: Buffer is smaller than size of file %s, %d>%d\n", 
						ifilename, entry->filelen, buffersize);
				return 0;
			}
		}
		fseek(m_fp,entry->filepos,SEEK_SET);
		fread(*ibuffer, entry->filelen, 1, m_fp);
		return entry->filelen;
	}
	return 0;
}

/*
===========================================
Build a StringList and return
===========================================
*/
int  CPakFile::GetFileList (StringList &strlst, 
							const char * path,
							const char *ext)
{
	if(!m_files)
		return 0;
	int matched = 0;

	if(path)
	{
		int plen = strlen(path);
		for(int i=0;i<m_numFiles;i++)
		{
			//Win32 specific
			if((_strnicmp(path,m_files[i]->filename,plen) == 0) &&
			   (!ext || Util::CompareExts(m_files[i]->filename,ext)))
			{
				strlst.push_back(std::string(m_files[i]->filename));
				matched ++;
			}
		}
	}
	else
	{
		for(int i=0;i<m_numFiles;i++)
		{
			if(!ext || Util::CompareExts(m_files[i]->filename,ext))
			{
				strlst.push_back(std::string(m_files[i]->filename));
				matched ++;
			}
		}
	}
	return matched;
}

/*
===========================================
Print List of m_files
===========================================
*/
void CPakFile::ListFiles()
{
	if(!m_files)
		return;
	for(int i=0;i< m_numFiles;i++)
		ComPrintf("%s\n",m_files[i]->filename);
}

/*
===========================================
Searches for PakEntry 
sets pointer, and returns true if finds it
===========================================
*/
bool CPakFile::BinarySearchForEntry(const char *name,	
									PakEntry_t ** item,
									int low, int high,
									int nameoffset)
{
	//Array doenst have any items
	if(low == high)
	{
		item = 0;
		return false;
	}
	
	int mid = (low+high)/2;
	int ret = _strnicmp(name,m_files[mid]->filename,nameoffset);

	if(ret == 0)		//name is equal to entry in array
	{
		*item = m_files[mid];
		return true;
	}
	else if(mid == low)
		return 0;
	else if(ret > 0)	//name is greater than array entry
		return BinarySearchForEntry(name,item,mid,high,nameoffset);
	else if(ret < 0)	//name is less than array entry
		return BinarySearchForEntry(name,item,low,mid,nameoffset);
	return false;
}




/*
==========================================
"open" a filestream and return handle
==========================================
*/

HFS CPakFile::OpenFile(const char *ifilename)
{
	if(m_numOpenFiles >= CArchive::ARCHIVEMAXOPENFILES)
	{
		ComPrintf("CPakFile::OpenFile: Max files opened in %s\n", m_archiveName);
		return -1;
	}

	//Find free space
	for(int i=0;i<CArchive::ARCHIVEMAXOPENFILES;i++)
	{
		if(m_openFiles[i].file == 0)
			break;
	}
	if(m_openFiles[i].file != 0)
	{
		ComPrintf("CPakFile::OpenFile: Unable to find unused file handle in %s\n", m_archiveName);
		return -1;
	}

	//Finds file, and adds to openFiles list if successful
	PakEntry_t * entry= 0;
	if(BinarySearchForEntry(ifilename,&entry,0,m_numFiles))
	{
		m_openFiles[i].file = entry;
		m_openFiles[i].curpos= 0;
		m_numOpenFiles++;
		return i;
	}
	return -1;
}

/*
==========================================
Close file
==========================================
*/
void CPakFile::CloseFile(HFS handle)
{
	if(m_openFiles[handle].file)
	{
		m_openFiles[handle].file = 0;
		m_openFiles[handle].curpos = 0;
		m_numOpenFiles--;
	}
}

/*
==========================================
Read from the given handle
returns bytes read
==========================================
*/
uint CPakFile::Read(void * buf, uint size, uint count, HFS handle)
{
	if(!buf || !size || !count)
	{
		ComPrintf("CPakFile::Read: Invalid parameters :%s\n",
							m_openFiles[handle].file->filename);
		return 0;
	}
	
	uint bytes_req = size * count;

	if(m_openFiles[handle].curpos + bytes_req > m_openFiles[handle].file->filelen)
	{
		ComPrintf("CPakFile::Read: FilePointer will overflow for given parms, %s\n", 
			m_openFiles[handle].file->filename);
		return 0;
	}

	fseek(m_fp, m_openFiles[handle].curpos + m_openFiles[handle].file->filepos, SEEK_SET);
	
	int items_read = ::fread(buf,size,count,m_fp);
	if(items_read != count) 
		ComPrintf("CPakFile::Read: Warning, only read %d of %d items for %s\n",
					items_read, count, m_openFiles[handle].file->filename);	
	
	m_openFiles[handle].curpos += (size*items_read);
	return items_read;
}

/*
==========================================
return current character
==========================================
*/
int CPakFile::GetChar(HFS handle)
{
	if(m_openFiles[handle].curpos +1 <= m_openFiles[handle].file->filelen)
	{
		::fseek(m_fp, m_openFiles[handle].curpos + m_openFiles[handle].file->filepos, SEEK_SET);
		return ::fgetc(m_fp);
	}
	return EOF;
}


/*
==========================================
Update file Handle's current position with
requested offset
==========================================
*/
bool CPakFile::Seek(int offset, int origin, HFS handle)
{
	int newpos = 0; //m_openFiles[handle].file->filepos;
	switch(origin)
	{
	case SEEK_SET:
			if(offset > m_openFiles[handle].file->filelen)
				offset = m_openFiles[handle].file->filelen;
			newpos += offset;
			break;
	case SEEK_END:
			if(offset)	//should be negative
				offset = 0;
			newpos += (m_openFiles[handle].file->filelen + offset);
			break;
	case SEEK_CUR:
			if((offset + m_openFiles[handle].curpos)  > m_openFiles[handle].file->filelen)
				offset = m_openFiles[handle].file->filelen - m_openFiles[handle].curpos;
			newpos += (m_openFiles[handle].curpos + offset);
			break;
	default:
			ComPrintf("CPakFile::Seek: Bad origin specified %s\n", m_openFiles[handle].file->filename);
			return false;
	}

//	return(!::fseek(m_fp,newpos,SEEK_SET));

	int filepos = newpos + m_openFiles[handle].file->filepos;
	if(!::fseek(m_fp,filepos,SEEK_SET))
	{
		m_openFiles[handle].curpos = newpos;
		return true;
	}
	return false;
}


/*
==========================================
get file handles current pos
==========================================
*/
uint CPakFile::GetPos(HFS handle)
{	return m_openFiles[handle].curpos;
}

/*
==========================================
get size of file belonging to handle
==========================================
*/
uint CPakFile::GetSize(HFS handle)
{	return m_openFiles[handle].file->filelen;
}
