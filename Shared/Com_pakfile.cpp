#include "Com_pakfile.h"


//======================================================================================
//Private definations

#define PAKENTRYSIZE	64

//Pak file header
typedef struct
{
	char	id[4];
	int		dirofs;
	int		dirlen;
}PakHeader_t;

//======================================================================================
/*
===========================================
Constructor/Destructor
===========================================
*/
CPakFile::CPakFile()
{
	m_fp = 0;
	files = 0; 
	numfiles = 0;
}

CPakFile::~CPakFile()
{
	if(m_fp)
		fclose(m_fp);

	for(int i=0;i<numfiles;i++)
	{
		delete files[i];
		files[i] = 0;
	}
	delete [] files;
}

/*
===========================================
Builds the filelist
===========================================
*/
bool CPakFile::Init(const char * base, const char * archive)
{
	//Open File
	char filepath[COM_MAXPATH];
	sprintf(filepath,"%s/%s",base,archive);

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

	//Find num files, and alloc space
	numfiles = phead.dirlen / PAKENTRYSIZE;

	//Seek to the fileinfo offset
	fseek(m_fp,	phead.dirofs, SEEK_SET);

#if 0
	//Copy to our list
	CPtrList<CPakEntry> * filelist = new CPtrList<CPakEntry>;
	CPtrList<CPakEntry> * iterator = filelist;
	for(int i=0;i<numfiles;i++)
	{
		iterator->item = new CPakEntry();
		fread(iterator->item, sizeof(CPakEntry),1,m_fp);
		iterator->next = new CPtrList<CPakEntry>;
		iterator = iterator->next;
	}
	//Sort the List
	QuickSortFileEntries(filelist,numfiles);

	//Copy to our array
	files = new CPakEntry * [numfiles];
	iterator = filelist;
	for(i=0;i<numfiles;i++)
	{
		files[i] = iterator->item;
		iterator->item = 0;
		iterator = iterator->next;
	}

	iterator = 0;
	delete filelist;

#else		//This way seems faster
	
	CPakEntry * temp = 0;
	files = new CPakEntry * [numfiles];
	
	for(int i= 0; i< numfiles;i++)
	{
		temp = new CPakEntry();
		fread(temp,sizeof(CPakEntry),1,m_fp);
		files[i] = temp;
	}
	QuickSortFileEntries(files,numfiles);

#endif

	strcpy(archivename,archive);
	ComPrintf("%s, Added %d files\n", archivename,numfiles);
	return true;
}

/*
===========================================
FIXME- 
this will have to use a custom memory manager later
on to alloc the buffer for the file.
===========================================
*/
long CPakFile::OpenFile(const char* path, byte ** buffer)
{
	if(*buffer)
	{
		ComPrintf("CPakFile::OpenFile: Expecting empty file pointer %s\n", path);
		return 0;
	}
	
	CPakEntry *	entry;

	if(BinarySearchForEntry(path,files,&entry,0,numfiles))
	{
		entry;
		*buffer= new unsigned char [entry->filelen];
		fseek(m_fp,entry->filepos,SEEK_SET);
		fread(*buffer, entry->filelen, 1, m_fp);
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
		ComPrintf("CPakFile::GetFileList: List needs to be deleted!, %s\n", archivename);
		return false;
	}
	
	if(!files)
		return false;

	list = new CStringList;
	CStringList  *iterator = list;

	for(int i = 0; i< numfiles; i++)
	{
		strcpy(iterator->string,files[i]->filename);
		
		iterator->next = new CStringList;
		iterator = iterator->next;
	}
	return false;
}


/*
===========================================
Print List of files
===========================================
*/
void CPakFile::ListFiles()
{
	if(!files)
		return;

	for(int i=0;i< numfiles;i++)
		ComPrintf("%s\n",files[i]->filename);
}


/*
===========================================
QuickSort PakEntry array
===========================================
*/

void CPakFile::QuickSortFileEntries(CPakEntry ** list, const int numitems)
{
	if(numitems < 2)
		return;

	int maxindex = numitems-1;
	int left=0;
	int right = maxindex; 
	CPakEntry ** sorted = new CPakEntry * [numitems];

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
	if((numitems - (right+1)) > 1)
		QuickSortFileEntries(&sorted[left+1],(numitems - (right+1)));		//starting from the one right after the pivot

	for(i=0;i<numitems;i++)
	{
		list[i] = sorted[i];
		sorted[i] = 0;
	}
	delete [] sorted;
}


/*
===========================================
QuickSorts PtrLst of CPakEntry pointers
===========================================
*/
void CPakFile::QuickSortFileEntries(CPtrList<CPakEntry> * list, const int numitems)
{
	if(numitems < 2)
		return;

	CPtrList<CPakEntry> * sorted = new CPtrList<CPakEntry>[numitems];	//dest array
	CPtrList<CPakEntry> * pivot = list;									//let the first one be the pivot
	CPtrList<CPakEntry> * iterator = list->next;					
	
	int maxindex = numitems-1;
	int left=0;
	int right = maxindex; 
	
	//loop one less time since the first item is the pivot
	for(int i=0,comp=0;i<maxindex;i++)
	{
		comp = _stricmp(iterator->item->filename,pivot->item->filename);
		
		if(comp < 0)
		{
			sorted[left].item = iterator->item;
			sorted[left].next = &sorted[(left+1)];
			left++;
		}
		else if(comp >= 0)
		{
			sorted[right].item = iterator->item;
			sorted[(right-1)].next = &sorted[right];
			right--;
		}
		iterator = iterator->next;
	}
	
	//Copy the pivot point in the empty space
	sorted[left].item = pivot->item;
	if(right == maxindex)
		sorted[left].next = 0;
	else
		sorted[left].next = &sorted[(left+1)];
		
	if(left > 1) 
		QuickSortFileEntries(sorted,left);								
	if((numitems - (right+1)) > 1)
		QuickSortFileEntries(&sorted[left+1],(numitems - (right+1)));		//starting from the one right after the pivot
	
	//List is sorted, copy into return filelist now
	//everything is sorted now copy it over
	iterator = list;
	for(i=0;i<numitems;i++)
	{
		iterator->item = sorted[i].item;
		iterator=iterator->next;
		sorted[i].next = 0;						//get rid of the links so we dont have problems deleting the array
		sorted[i].item = 0;
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
									CPakEntry ** array, 
									CPakEntry ** item,
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