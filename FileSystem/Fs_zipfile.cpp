#include "Fs_zipfile.h"


#define CENTRAL_HDR_SIG	'\001','\002'	/* the infamous "PK" signature bytes, */
#define LOCAL_HDR_SIG	'\003','\004'	/*  sans "PK" (so unzip executable not */
#define END_CENTRAL_SIG	'\005','\006'	/*  mistaken for zipfile itself) */
#define EXTD_LOCAL_SIG	'\007','\010'	/* [ASCII "\113" == EBCDIC "\080" ??] */

#define MAXCOMMENTBUFFERSIZE			1024
#define ZIP_LOCAL_FILE_HEADER_SIZE      26

static const char zip_hdr_central[4] = { 'P', 'K', CENTRAL_HDR_SIG };
static const char zip_hdr_local[4] = { 'P', 'K', LOCAL_HDR_SIG };
static const char zip_hdr_endcentral[4] = { 'P', 'K', END_CENTRAL_SIG };
static const char zip_hdr_extlocal[4] = { 'P', 'K', EXTD_LOCAL_SIG };

typedef struct ZIP_local_file_header_s 
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
} ZIP_local_file_header;

typedef struct ZIP_central_directory_file_header_s 
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
} ZIP_central_directory_file_header;

typedef struct ZIP_end_central_dir_record_s 
{
	ushort	number_this_disk;
	ushort	num_disk_start_cdir;
	ushort	num_entries_centrl_dir_ths_disk;
	ushort	total_entries_central_dir;
	ulong	size_central_directory;
	ulong	offset_start_central_directory;
	ushort	zipfile_comment_length;

} ZIP_end_central_dir_record;



struct CZipFile::ZipEntry_t
{
	ZipEntry_t() { filepos = filelen = 0; };
	char filename[COM_MAXFILENAME];
	ulong filepos, 
		 filelen;
};

struct CZipFile::ZipOpenFile_t : public CZipFile::ZipEntry_t
{
	ZipOpenFile_t() { curpos = 0; };
	ulong curpos;
};



static void getShort (FILE* fin, ushort &is)
{
    int ix=0;
    ix = fgetc(fin);
	ix += (((ulong)fgetc(fin)) << 8);
	is = ix;
}

static void getLong (FILE* fin, ulong &ix)
{
    int i;

    i = fgetc(fin);
    ix = (ulong)i;
    
    i = fgetc(fin);
    ix += ((ulong)i)<<8;

    i = fgetc(fin);
    ix += ((ulong)i)<<16;

    i = fgetc(fin);
    ix += ((ulong)i)<<24;
}



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

	memset(m_openFiles,sizeof(ZipOpenFile_t *) * ARCHIVEMAXOPENFILES, 0);
	m_numOpenFiles = 0;
}

CZipFile::~CZipFile()
{
	if(m_fp)
		fclose(m_fp);

	memset(m_openFiles,sizeof(ZipOpenFile_t *) * ARCHIVEMAXOPENFILES, 0);
	m_numOpenFiles = 0;

	for(int i=0;i<m_numFiles;i++)
	{
		if(m_files[i])
		{
			delete m_files[i];
			m_files[i] = 0;
		}
	}
	delete [] m_files;
}


/*
==========================================
Initialize the zip file
Open it, get info
==========================================
*/
bool CZipFile::Init(const char * archivepath, const char * basepath)
{
	//Open File
	char filepath[COM_MAXPATH];
	sprintf(filepath,"%s/%s",basepath,archivepath);

	if((m_fp = ::fopen(filepath, "r+b")) == 0)
	{
		ComPrintf("CZipFile::Init: Unable to open %s\n",filepath);
		return false;
	}

	ulong central_pos = GetCentralDirOffset(m_fp);

	if(central_pos == 0)
	{
		ComPrintf("CZipFile::Init: Error finding central dir: %s\n", filepath);
		fclose(m_fp);
		return false;
	}

	if(fseek(m_fp,central_pos,SEEK_SET) !=0)
	{
		ComPrintf("CZipFile::Init: Error seeking to Central Dir %s\n", filepath);
		fclose(m_fp);
		return false;
	}

	ulong sig=0;
	ushort numdisk=0, numentry=0, numfiles =0, numTotFiles = 0;

	getLong(m_fp,sig);
	getShort(m_fp, numdisk);
	getShort(m_fp, numentry);
	getShort(m_fp, numfiles);
	getShort(m_fp, numTotFiles);

	if(numfiles != numTotFiles ||
	   numdisk  > 0 ||
	   numentry > 0 )
	{
	   ComPrintf("CZipFile::Init: Error Bad zip file %s\n", filepath);
	   fclose(m_fp);
	   return false;
    }


	ulong cdirSize = 0 , cdirOffset = 0;
	ushort cdirCommentSize = 0;

	getLong(m_fp,cdirSize);
	getLong(m_fp,cdirOffset);
	getShort(m_fp,cdirCommentSize);

	if(central_pos < cdirOffset + cdirSize)
	{
		ComPrintf("CZipFile::Init: Error Bad offset position %s\n", filepath);
		fclose(m_fp);
		return 0;
	}

	fseek(m_fp,cdirOffset,SEEK_SET);

	strcpy(m_archiveName,archivepath);

	return BuildZipEntriesList(m_fp, numfiles);
//	return true;
}


ulong CZipFile::GetCentralDirOffset(FILE * fin)
{
	unsigned char* buf=0;
	ulong uSizeFile=0;
	ulong uBackRead=0;
	ulong uMaxBack =0xffff; /* maximum size of global comment */
	ulong uPosFound=0;

	
	if (fseek(fin,0,SEEK_END) != 0)
		return 0;

	uSizeFile = ftell( fin );

	if (uMaxBack > uSizeFile)
		uMaxBack = uSizeFile;

	buf = (unsigned char*)malloc(MAXCOMMENTBUFFERSIZE+4);
	if (buf==NULL)
		return 0;

	uBackRead = 4;

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
	free(buf);
	return uPosFound;
}



bool CZipFile::BuildZipEntriesList(FILE * fp, int numfiles)
{
	char	bufname[COM_MAXFILENAME];	
	char	bufhdr[5];
	ZIP_central_directory_file_header	cdfh;
	bool	err = false;
	size_t	curpos = 0;

	m_numFiles = 0;
	m_files = new ZipEntry_t * [numfiles];
	
	for (int i=0;i<numfiles;i++) 
	{
		if(!fread(bufhdr,sizeof(zip_hdr_central),1,fp) ||
			memcmp(bufhdr,zip_hdr_central,sizeof(zip_hdr_central)))
		{
			ComPrintf("CZipFile::BuildZipEntriesList: Bad CentralDir Sig %s\n", m_archiveName);
			
			for(i=0;i<m_numFiles;i++)
				m_files[i] = 0;
			m_numFiles = 0;
			delete [] m_files;
			
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
		   (cdfh.filename_length < COM_MAXFILENAME) &&
		   (fread(bufname, cdfh.filename_length, 1, fp)) &&
		   (bufname[cdfh.filename_length - 1] != '/'))
		{
			bufname[cdfh.filename_length] = '\0';
			ComPrintf("%s\n", bufname);

			m_files[m_numFiles] = new ZipEntry_t();
			strcpy(m_files[m_numFiles]->filename,bufname);
			m_files[m_numFiles]->filelen = cdfh.ucsize;
			m_files[m_numFiles]->filepos = cdfh.relative_offset_local_header + 
								 sizeof(zip_hdr_local) + 
								 ZIP_LOCAL_FILE_HEADER_SIZE +
								 cdfh.filename_length + 
								 cdfh.extra_field_length;
			m_numFiles++;

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
bool  CZipFile::GetFileList (CStringList * list)
{
	if(list)
	{
		ComPrintf("CZipFile::GetFileList: List needs to be deleted!, %s\n", m_archiveName);
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


bool CZipFile::HasFile(const char * filename)
{	return false;
}


/*
==========================================
Load a file in the zip to the given buffer
==========================================
*/
uint CZipFile::LoadFile(byte ** ibuffer, 
						uint buffersize, 
						const char *ifilename)
{	return 0;
}





HFS CZipFile::OpenFile(const char *ifilename)
{
	return 0;
}

void CZipFile::CloseFile(HFS handle)
{
}

ulong CZipFile::Read(void * buf, uint size, uint count, HFS handle)
{
	return 0;
}

int  CZipFile::GetChar(HFS handle)
{
	return 0;
}

bool CZipFile::Seek(uint offset, int origin, HFS handle)
{
	return false;
}

uint CZipFile::GetPos(HFS handle)
{
	return 0;
}

uint CZipFile::GetSize(HFS handle)
{
	return 0;
}




















#if 0


static bool read_cdfh(ZIP_central_directory_file_header * cdfh, FILE * infile)
{
	byte buff[ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE];

	if (!fread(buff, ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE, 1, infile))// < ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE) 
	{
		printf("unable to read cdir\n");
		return false;
	}

	cdfh->version_made_by[0]			= buff[C_VERSION_MADE_BY_0];
	cdfh->version_made_by[1]			= buff[C_VERSION_MADE_BY_1];
	cdfh->version_needed_to_extract[0]	= buff[C_VERSION_NEEDED_TO_EXTRACT_0];
	cdfh->version_needed_to_extract[1]	= buff[C_VERSION_NEEDED_TO_EXTRACT_1];
	cdfh->general_purpose_bit_flag		= ShortSwap(*(short *)(buff + C_GENERAL_PURPOSE_BIT_FLAG));
	cdfh->compression_method			= ShortSwap(*(short *)(buff + C_COMPRESSION_METHOD));
	cdfh->last_mod_file_time			= ShortSwap(*(short *)(buff + C_LAST_MOD_FILE_TIME));
	cdfh->last_mod_file_date			= ShortSwap(*(short *)(buff + C_LAST_MOD_FILE_DATE));
	cdfh->crc32							= LongSwap(*(long *)(buff + C_CRC32));
	cdfh->csize							= LongSwap(*(long *)(buff + C_COMPRESSED_SIZE));
	cdfh->ucsize						= LongSwap(*(long *)(buff + C_UNCOMPRESSED_SIZE));
	cdfh->filename_length				= ShortSwap(*(short *)(buff + C_FILENAME_LENGTH));
	cdfh->extra_field_length			= ShortSwap(*(short *)(buff + C_EXTRA_FIELD_LENGTH));
	cdfh->file_comment_length			= ShortSwap(*(short *)(buff + C_FILE_COMMENT_LENGTH));
	cdfh->disk_number_start				= ShortSwap(*(short *)(buff + C_DISK_NUMBER_START));
	cdfh->internal_file_attributes		= ShortSwap(*(short *)(buff + C_INTERNAL_FILE_ATTRIBUTES));
	cdfh->external_file_attributes		= LongSwap(*(long *)(buff + C_EXTERNAL_FILE_ATTRIBUTES));
	cdfh->relative_offset_local_header	= LongSwap(*(long *)(buff + C_RELATIVE_OFFSET_LOCAL_HEADER));
	return true;
}


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