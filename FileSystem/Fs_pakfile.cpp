#include "Fs_pakfile.h"

//======================================================================================
//Private definations
//======================================================================================

#define PAKENTRYSIZE	64

//Pak file header
typedef struct
{
	char	id[4];
	int		dirofs;
	int		dirlen;
}PakHeader_t;


struct CPakFile::PakEntry_t
{
	PakEntry_t() { filepos = filelen = 0; };
	char filename[56];
	long filepos, 
		 filelen;
};

//======================================================================================
//======================================================================================

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
}

CPakFile::~CPakFile()
{
	if(m_fp)
		fclose(m_fp);

	for(int i=0;i<m_numFiles;i++)
	{
		delete m_files[i];
		m_files[i] = 0;
	}
	delete [] m_files;
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

	//Seek to the fileinfo offset
	fseek(m_fp,	phead.dirofs, SEEK_SET);

	PakEntry_t * temp= 0;
	for(int i= 0; i< m_numFiles;i++)
	{
		temp = new PakEntry_t();
		fread(temp,sizeof(PakEntry_t),1,m_fp);
		m_files[i] = temp;
	}
	QuickSortFileEntries(m_files,m_numFiles);

	strcpy(m_archiveName,archivepath);
	ComPrintf("%s, Added %d entries\n", m_archiveName,m_numFiles);
	return true;
}

/*
===========================================
FIXME- 
this will have to use a custom memory manager later
on to alloc the buffer for the file.
===========================================
*/
uint CPakFile::LoadFile(byte ** ibuffer, uint &buffersize, 
							bool staticbuffer, const char *ifilename)
{
	if(!staticbuffer && *ibuffer)
	{
		ComPrintf("CPakFile::OpenFile: Expecting empty file pointer %s\n", ifilename);
		return 0;
	}

	PakEntry_t * entry=0;
	if(BinarySearchForEntry(ifilename,m_files,&entry,0,m_numFiles))
	{
		if(!staticbuffer)
		{
			*ibuffer= (byte*)MALLOC(entry->filelen);
		}
		else
		{
			if(entry->filelen > buffersize)
			{
				free(*ibuffer);
				*ibuffer= (byte*)MALLOC(entry->filelen);
				buffersize = entry->filelen;
			}
		}
		//*buffer= new unsigned char [entry->filelen];
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
bool CPakFile::GetFileList (CStringList * list)
{
	if(list)
	{
		ComPrintf("CPakFile::GetFileList: List needs to be deleted!, %s\n", m_archiveName);
		return false;
	}
	
	if(!m_files)
		return false;

	list = new CStringList();
	CStringList  *iterator = list;

	for(int i = 0; i< m_numFiles; i++)
	{
		strcpy(iterator->string,m_files[i]->filename);
		iterator->next = new CStringList;
		iterator = iterator->next;
	}
	return true;
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
QuickSort PakEntry array
===========================================
*/
void CPakFile::QuickSortFileEntries(PakEntry_t ** list, const int numitems)
{
	if(numitems < 2)
		return;

	int maxindex = numitems-1;
	int left=0;
	int right = maxindex; 
	PakEntry_t ** sorted = new PakEntry_t * [numitems];

	for(int i=1,comp=0;i<=maxindex;i++)
	{
		comp = _stricmp(list[i]->filename,list[0]->filename);
		
		if(comp < 0)
		{
			sorted[left] = list[i];
			left++;
		}
		else if(comp >= 0)
		{
			sorted[right] = list[i];
			right--;
		}
	}

	//Copy the pivot point in the empty space
	sorted[left] = list[0];
	if(left > 1) 
		QuickSortFileEntries(sorted,left);								
	//starting from the one right after the pivot
	if((numitems - (right+1)) > 1)
		QuickSortFileEntries(&sorted[left+1],(numitems - (right+1)));		

	for(i=0;i<numitems;i++)
	{
		list[i] = sorted[i];
		sorted[i] = 0;
	}
	delete [] sorted;
}

/*
===========================================
Searches for PakEntry 
sets pointer, and returns true if finds it
===========================================
*/
bool CPakFile::BinarySearchForEntry(const char *name,	
									PakEntry_t ** array, 
									PakEntry_t ** item,
									int low, int high)
{
	//Array doenst have any items
	if(low == high)
	{
		item = 0;
		return false;
	}
	
	int mid = (low+high)/2;
	int ret = _stricmp(name,array[mid]->filename);

	if(ret == 0)		//name is equal to entry in array
	{
		*item = array[mid];
		return true;
	}
	else if(mid == low)
		return 0;
	else if(ret > 0)	//name is greater than array entry
		return BinarySearchForEntry(name,array,item,mid,high);
	else if(ret < 0)	//name is less than array entry
		return BinarySearchForEntry(name,array,item,low,mid);
	return false;
}