#include "Fs_zipfile.h"
#include "I_file.h"

/*
======================================================================================
Private Declarations and definitions

Zip extraction code was intiallly based on the zlib sources

Questions about zlib should be sent to <zlib@quest.jpl.nasa.gov>, or to
Gilles Vollant <info@winimage.com> for the Windows DLL version.
The zlib home page is http://www.cdrom.com/pub/infozip/zlib/
The official zlib ftp site is ftp://ftp.cdrom.com/pub/infozip/zlib/

======================================================================================
*/

#define CENTRAL_HDR_SIG	'\001','\002'
#define LOCAL_HDR_SIG	'\003','\004'
#define END_CENTRAL_SIG	'\005','\006'
#define EXTD_LOCAL_SIG	'\007','\010'

namespace
{
	enum
	{
		MAXCOMMENTBUFFERSIZE		= 1024,
		ZIP_LOCAL_FILE_HEADER_SIZE  = 26
	};

	struct ZIP_local_file_header 
	{
		byte	version_needed_to_extract[2];
		ushort	general_purpose_bit_flag;
		ushort	compression_method;
		ushort	last_mod_file_time;
		ushort	last_mod_file_date;
		ulong	crc32;
		ulong	csize;
		ulong	ucsize;
		ushort	filename_length;
		ushort	extra_field_length;
	};

	struct ZIP_central_directory_file_header 
	{
		byte	version_made_by[2];
		byte	version_needed_to_extract[2];
		ushort	general_purpose_bit_flag;
		ushort	compression_method;
		ushort	last_mod_file_time;
		ushort	last_mod_file_date;
		ulong	crc32;
		ulong	csize;
		ulong	ucsize;
		ushort	filename_length;
		ushort  extra_field_length;
		ushort  file_comment_length;
		ushort  disk_number_start;
		ushort  internal_file_attributes;
		ulong	external_file_attributes;
		ulong	relative_offset_local_header;
	};

	struct ZIP_end_central_dir_record 
	{
		ushort	number_this_disk;
		ushort	num_disk_start_cdir;
		ushort	num_entries_centrl_dir_ths_disk;
		ushort	total_entries_central_dir;
		ulong	size_central_directory;
		ulong	offset_start_central_directory;
		ushort	zipfile_comment_length;

	};

	//Zip header signitures
	const char zip_hdr_central[4] = { 'P', 'K', CENTRAL_HDR_SIG };
	const char zip_hdr_local[4] = { 'P', 'K', LOCAL_HDR_SIG };
	const char zip_hdr_endcentral[4] = { 'P', 'K', END_CENTRAL_SIG };
	const char zip_hdr_extlocal[4] = { 'P', 'K', EXTD_LOCAL_SIG };

	//Utility funcs to read ZipHeader data in correcy byte order
	inline void getShort (FILE* fin, ushort &is)
	{
		int	ix = fgetc(fin);
		ix += (((ulong)fgetc(fin)) << 8);
		is = ix;
	}

	inline void getLong (FILE* fin, ulong &ix)
	{
		int i = fgetc(fin);
		ix = (ulong)i;
    
		i = fgetc(fin);
		ix += ((ulong)i)<<8;

		i = fgetc(fin);
		ix += ((ulong)i)<<16;

		i = fgetc(fin);
		ix += ((ulong)i)<<24;
	}
}

//======================================================================================
//======================================================================================

/*
==========================================
Constructor/Destructor
==========================================
*/
CZipFile::CZipFile()
{
	m_fp = 0;
	m_files = 0; 
	m_numFiles = 0;

	memset(m_openFiles,0, sizeof(ZipOpenFile_t) * CArchive::ARCHIVEMAXOPENFILES);
	m_numOpenFiles = 0;
}

CZipFile::~CZipFile()
{
	if(m_fp)
		fclose(m_fp);

	memset(m_openFiles,0, sizeof(ZipOpenFile_t) * CArchive::ARCHIVEMAXOPENFILES);
	m_numOpenFiles = 0;

	if(m_files)
	{
		for(int i=0;i<m_numFiles;i++)
		{
			if(m_files[i])
			{
				delete m_files[i];
				m_files[i] = 0;
			}
		}
		delete [] m_files;
		m_files = 0;
	}
}


/*
==========================================
Initialize the zip file
Open it, get file info, build file list
==========================================
*/
bool CZipFile::Init(const char * archivepath, const char * basepath)
{
	//Open File
	char filepath[COM_MAXPATH];
	sprintf(filepath,"%s/%s",basepath,archivepath);

	if((m_fp = ::fopen(filepath, "rb")) == 0)
	{
		ComPrintf("CZipFile::Init: Unable to open %s\n",filepath);
		return false;
	}

	//Get Last Directory Offset and validate it
	ulong last_dir_offset = GetLastRecordOffset(m_fp);
	if(last_dir_offset == 0)
	{
		ComPrintf("CZipFile::Init: Error finding central dir: %s\n", filepath);
		fclose(m_fp);
		return false;
	}

	//Seek to Central directoy offset
	if(fseek(m_fp,last_dir_offset,SEEK_SET) !=0)
	{
		ComPrintf("CZipFile::Init: Error seeking to Central Dir %s\n", filepath);
		fclose(m_fp);
		return false;
	}

	//Get Global zip archive data
	ulong sig=0;
	ushort numdisk=0, numentry=0, numfiles =0, numTotFiles = 0;

	getLong(m_fp,sig);
	getShort(m_fp, numdisk);
	getShort(m_fp, numentry);
	getShort(m_fp, numfiles);
	getShort(m_fp, numTotFiles);

	//Make sure archive doesnt span disks
	if(numfiles != numTotFiles ||
	   numdisk  > 0 ||
	   numentry > 0 )
	{
	   ComPrintf("CZipFile::Init: Error Bad zip file %s\n", filepath);
	   fclose(m_fp);
	   return false;
    }

	//Get Central directory information
	ulong cdirSize = 0 , cdirOffset = 0;
	ushort cdirCommentSize = 0;

	getLong(m_fp,cdirSize);
	getLong(m_fp,cdirOffset);
	getShort(m_fp,cdirCommentSize);

	if(last_dir_offset < cdirOffset + cdirSize)
	{
		ComPrintf("CZipFile::Init: Error Bad offset position %s\n", filepath);
		fclose(m_fp);
		return 0;
	}

	//Seek to the first "file record" offset
	fseek(m_fp,cdirOffset,SEEK_SET);
	
	strcpy(m_archiveName,archivepath);

	//Build list of Files in the Zip by reading all the file records
	return BuildZipEntriesList(m_fp, numfiles);
}


/*
==========================================
Looks for the End of File Directory record
of the zip. Starts from the end of the file,
and seeks backwords until it find the signature
==========================================
*/
ulong CZipFile::GetLastRecordOffset(FILE * fin)
{
	if (fseek(fin,0,SEEK_END) != 0)
		return 0;

	
	byte  buf[MAXCOMMENTBUFFERSIZE+4];
	ulong uBackRead=4;
	ulong uMaxBack =0xffff; // maximum size of global comment
	ulong uPosFound=0;
	ulong uSizeFile = ftell( fin );

	if (uMaxBack > uSizeFile)
		uMaxBack = uSizeFile;

	while (uBackRead< uMaxBack)
	{
		ulong uReadSize,uReadPos;
		
		if (uBackRead+MAXCOMMENTBUFFERSIZE>uMaxBack) 
			uBackRead = uMaxBack;
		else
			uBackRead+=MAXCOMMENTBUFFERSIZE;
		
		uReadPos = uSizeFile-uBackRead;
		
		uReadSize = ((MAXCOMMENTBUFFERSIZE+4) < (uSizeFile-uReadPos)) ? 
					 (MAXCOMMENTBUFFERSIZE+4) : (uSizeFile-uReadPos);
		
		if (fseek(fin,uReadPos,SEEK_SET)!=0)
			break;

		if (fread(buf,(uint)uReadSize,1,fin)!=1)
			break;

        for (int i=(int)uReadSize-3; (i--)>0; )
		{
			if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
			{
				uPosFound = uReadPos+i;
				break;
			}
		}

		if(uPosFound!=0)
			break;
	}
	return uPosFound;
}

/*
==========================================
Read consecutive File Records, get filenames 
and their offsets from them, then
build an ordered list with that info for quick access
==========================================
*/
bool CZipFile::BuildZipEntriesList(FILE * fp, int numfiles)
{
	char	bufname[COM_MAXFILENAME];	
	char	bufhdr[5];
	bool	err = false;
	int		i,j,destIndex=0;
	size_t	curpos = 0;
	ZIP_central_directory_file_header	cdfh;

	m_numFiles = 0;
	m_files = new ZipEntry_t * [numfiles];


	for (i=0;i<numfiles;i++) 
	{
		if(!fread(bufhdr,sizeof(zip_hdr_central),1,fp) ||
			memcmp(bufhdr,zip_hdr_central,sizeof(zip_hdr_central)))
		{
			ComPrintf("CZipFile::BuildZipEntriesList: Bad CentralDir Sig %s\n", m_archiveName);
			return false;
		}

		cdfh.version_made_by[0] = fgetc(fp);
		cdfh.version_made_by[1]	= fgetc(fp);
		cdfh.version_needed_to_extract[0]	= fgetc(fp);
		cdfh.version_needed_to_extract[1]	= fgetc(fp);
		
		getShort(fp,cdfh.general_purpose_bit_flag);
		getShort(fp,cdfh.compression_method);
		getShort(fp,cdfh.last_mod_file_time	);
		getShort(fp,cdfh.last_mod_file_date);
		getLong(fp,cdfh.crc32);	
		getLong(fp,cdfh.csize);	
		getLong(fp,cdfh.ucsize);
		getShort(fp,cdfh.filename_length);
		getShort(fp,cdfh.extra_field_length	);
		getShort(fp,cdfh.file_comment_length);
		getShort(fp,cdfh.disk_number_start);
		getShort(fp,cdfh.internal_file_attributes);
		getLong(fp,cdfh.external_file_attributes);
		getLong(fp,cdfh.relative_offset_local_header);


		//Validate entry. don't add if name is too long, or can't read name
		//Only add FILE entries. and Uncompressed files at that.
		if((cdfh.csize == cdfh.ucsize) &&
		   (cdfh.filename_length < COM_MAXFILENAME))
		{
			if(fread(bufname, cdfh.filename_length, 1, fp) &&
			  (bufname[cdfh.filename_length - 1] != '/'))
			{
				bufname[cdfh.filename_length] = '\0';
//				ComPrintf("%s\n", bufname);

				ZipEntry_t * newfile = new ZipEntry_t();
				strcpy(newfile->filename,bufname);
				newfile->filelen = cdfh.ucsize;
				newfile->filepos = cdfh.relative_offset_local_header + 
								   sizeof(zip_hdr_local) + 
								   ZIP_LOCAL_FILE_HEADER_SIZE +
								   cdfh.filename_length + 
								   cdfh.extra_field_length;

				//Find a place to insert the new entry
				
				//default to last position
				destIndex = m_numFiles;	

				for(j=0;j<m_numFiles;j++)
				{
					if(!m_files[j])
						break;

					//If new filename is smaller then entry at current index, 
					//then shift all the entires forward to make space for new entry
					if(_stricmp(newfile->filename, m_files[j]->filename) < 0)
					{
						destIndex = j;

						//start from the last entry and
						//move all the old pointers forward by one
						j = m_numFiles;
						while(j > destIndex)
						{
							m_files[j] = m_files[j-1];
							j--;
						}
						break;
					}
				}
				m_files[destIndex] = newfile;
				m_numFiles++;
			}
		}

		if(fseek(fp,cdfh.extra_field_length + cdfh.file_comment_length, SEEK_CUR)) 
		{
			ComPrintf("CZipFile::BuildZipEntriesList: Unable to read entry: %s\n", m_archiveName);
			break;
		}
	}
	return true;
}

/*
==========================================
List all the files in the zip
==========================================
*/
void  CZipFile::ListFiles()
{
	if(!m_files)
		return;
	for(int i=0;i< m_numFiles;i++)
		ComPrintf("%s\n",m_files[i]->filename);
}

/*
==========================================
Get a list of files
==========================================
*/
int  CZipFile::GetFileList (StrList &strlst, 
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
==========================================
Check to see if the archive has the file
==========================================
*/
bool CZipFile::FindFile(char * buf, int buflen,const char * filename)
{
	ZipEntry_t * entry=0;
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
==========================================
Search for given file name
==========================================
*/
bool CZipFile::BinarySearchForEntry(const char *name,	
									ZipEntry_t ** item,
									int low, int high,int nameoffset)
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
Load the requested file into the given buffer
==========================================
*/
uint CZipFile::LoadFile(byte ** ibuffer, 
						uint buffersize, 
						const char *ifilename)
{
	if(!buffersize && *ibuffer)
	{
		ComPrintf("CZipFile::OpenFile: Expecting empty file pointer %s\n", ifilename);
		return 0;
	}

	ZipEntry_t * entry=0;
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
				ComPrintf("CZipFile::LoadFile: Buffer is smaller than size of file %s, %d>%d\n", 
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
==========================================
Open a file and return a handle
==========================================
*/
HFS CZipFile::OpenFile(const char *ifilename)
{
	if(m_numOpenFiles >= CArchive::ARCHIVEMAXOPENFILES)
	{
		ComPrintf("CZipFile::OpenFile: Max files opened in %s\n", m_archiveName);
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
		ComPrintf("CZipFile::OpenFile: Unable to find unused file handle in %s\n", m_archiveName);
		return -1;
	}
	//Finds file, and adds to openFiles list if successful
	ZipEntry_t * entry= 0;
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
Close the given handle
==========================================
*/
void CZipFile::CloseFile(HFS handle)
{
	if(handle >= 0 && m_openFiles[handle].file)
	{
		m_openFiles[handle].curpos = 0;
		m_openFiles[handle].file = 0;
		m_numOpenFiles--;
	}
}

/*
==========================================
Read from the given file
==========================================
*/
uint CZipFile::Read(void * buf, uint size, uint count, HFS handle)
{
	if(!buf || !size || !count)
	{
		ComPrintf("CZipFile::Read: Invalid parameters :%s\n",
							m_openFiles[handle].file->filename);
		return 0;
	}
	
	uint bytes_req = size * count;
	if(m_openFiles[handle].curpos + bytes_req > m_openFiles[handle].file->filelen)
	{
		ComPrintf("CZipFile::Read: FilePointer will overflow for given parms, %s\n", 
			m_openFiles[handle].file->filename);
		return 0;
	}

	fseek(m_fp, m_openFiles[handle].curpos + m_openFiles[handle].file->filepos, SEEK_SET);
	
	uint items_read = ::fread(buf,size,count,m_fp);
	if(items_read != count) 
		ComPrintf("CZipFile::Read: Warning, only read %d of %d items for %s\n",
					items_read, count, m_openFiles[handle].file->filename);	
	
	m_openFiles[handle].curpos += (items_read*size);
	return items_read;
}

/*
==========================================
Return char at current pos and advance
==========================================
*/
int  CZipFile::GetChar(HFS handle)
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
Seek to given pos within the file data
==========================================
*/
bool CZipFile::Seek(int offset, int origin, HFS handle)
{
	int newpos = 0;
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
			ComPrintf("CZipFile::Seek: Bad origin specified %s\n", m_openFiles[handle].file->filename);
			return false;
	}

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
Get current pos in the file for the handle
==========================================
*/
uint CZipFile::GetPos(HFS handle)
{	return m_openFiles[handle].curpos;
}

/*
==========================================
Get size of file for this handle
==========================================
*/
uint CZipFile::GetSize(HFS handle)
{	return m_openFiles[handle].file->filelen;
}





//==========================================================================
//==========================================================================



#if 0

/*			curpos = ftell(m_fp);
			if(fseek(fp,cdfh.relative_offset_local_header + 
					sizeof(zip_hdr_local) + ZIP_LOCAL_FILE_HEADER_SIZE,
					SEEK_SET)==0)
			{
				fseek(fp,cdfh.filename_length + cdfh.extra_field_length, SEEK_CUR);

				char fname[128];
				sprintf(fname,"file%d.pcx",i);
				
				FILE * fout = fopen(fname,"w+b");
				if(fout)
				{
					byte  * outbuf = (byte*)malloc(cdfh.ucsize);
					fread(outbuf,cdfh.ucsize,1,fp);
					fwrite(outbuf,cdfh.ucsize,1,fout);
					fclose(fout);
					free(outbuf);
				}
			}
			fseek(fp,curpos,SEEK_SET);
*/

/*--- ZIP_local_file_header layout ---------------------------------------------*/
#define ZIP_LOCAL_FILE_HEADER_SIZE              26
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22
#      define L_EXTRA_FIELD_LENGTH              24

/*--- ZIP_central_directory_file_header layout ---------------------------------*/
#define ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE  42
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16
#      define C_UNCOMPRESSED_SIZE               20
#      define C_FILENAME_LENGTH                 24
#      define C_EXTRA_FIELD_LENGTH              26
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38

/*--- ZIP_end_central_dir_record layout ----------------------------------------*/
#define ZIP_END_CENTRAL_DIR_RECORD_SIZE         18
#      define E_NUMBER_THIS_DISK                0
#      define E_NUM_DISK_WITH_START_CENTRAL_DIR 2
#      define E_NUM_ENTRIES_CENTRL_DIR_THS_DISK 4
#      define E_TOTAL_ENTRIES_CENTRAL_DIR       6
#      define E_SIZE_CENTRAL_DIRECTORY          8
#      define E_OFFSET_START_CENTRAL_DIRECTORY  12
#      define E_ZIPFILE_COMMENT_LENGTH          16

#endif