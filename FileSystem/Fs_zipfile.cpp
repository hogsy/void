#include "Fs_zipfile.h"

/*
==========================================
Constructor/Destructor
==========================================
*/
CZipFile::CZipFile()
{
	m_hFile = 0;
}

CZipFile::~CZipFile()
{
    if (m_hFile)
		unzClose(m_hFile);
	m_hFile = 0;
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

	m_hFile = unzOpen(filepath);
	if(!m_hFile)
	{
		ComPrintf("CZipFile::Init: Unable to open %s\n", filepath);
		return false;
	}

	unz_global_info unzGlobal;
	if(!unzGetGlobalInfo(m_hFile,&unzGlobal) == UNZ_OK)
	{
		ComPrintf("CZipFile::Init: Unable to read %s\n", filepath);
		return false;
	}
	m_numFiles = unzGlobal.number_entry;
	strcpy(m_archiveName,archivepath);
	ComPrintf("%s, Added %d entries\n",archivepath, unzGlobal.number_entry);
	return true;
}

/*
==========================================
List all the files in the zip
==========================================
*/
void  CZipFile::ListFiles()
{
    int ret=0;
	char filename[64];

    ret = unzGoToFirstFile(m_hFile);
    while (ret == UNZ_OK)
    {
		unzGetCurrentFileInfo(m_hFile, NULL, filename, 64, NULL, 0, NULL, 0);
		ret = unzGoToNextFile(m_hFile);
		ComPrintf("%s\n",filename);
    }
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
	if(!m_hFile)
	{
		ComPrintf("CZipFile::GetFileList: No zipfile opened\n");
		return false;
	}

	char filename[64];
	int ret=0;
	list = new CStringList();
	CStringList  *iterator = list;

	ret = unzGoToFirstFile(m_hFile);
    while (ret == UNZ_OK)
    {
		unzGetCurrentFileInfo(m_hFile, NULL, filename, 64, NULL, 0, NULL, 0);
		ret = unzGoToNextFile(m_hFile);

		strcpy(iterator->string,filename);
		
		iterator->next = new CStringList;
		iterator = iterator->next;
    }
	return true;
}


bool CZipFile::HasFile(const char * filename)
{
	if (unzLocateFile(m_hFile,filename, 2) == UNZ_OK)
		return true;
	return false;
}


/*
==========================================
Load a file in the zip to the given buffer
==========================================
*/
uint CZipFile::LoadFile(byte ** ibuffer, 
						uint buffersize, 
						const char *ifilename)
{
	if (unzLocateFile(m_hFile,ifilename, 2) != UNZ_OK)
		return 0;
	
	if (unzOpenCurrentFile(m_hFile) != UNZ_OK)
	{
		ComPrintf("CZipFile::OpenFile: Unable to open %s , %s\n", ifilename, m_archiveName);
	    return 0;
	}

	unz_file_info fileinfo;
    if (unzGetCurrentFileInfo(m_hFile, &fileinfo, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
	{
		ComPrintf("CZipFile::OpenFile: Unable to get file info %s , %s\n", ifilename, m_archiveName);
		return 0;
	}
	
	if(fileinfo.uncompressed_size != fileinfo.compressed_size)
	{
		ComPrintf("CZipFile::OpenFile: File is compressed %s , %s\n", ifilename, m_archiveName);
		return 0;
	}

	uint size =  fileinfo.uncompressed_size;
	if(!buffersize)
	{
		*ibuffer = (byte*)MALLOC(size);
	}
	else
	{
		if(size > buffersize)
		{
			ComPrintf("CZipFile::LoadFile: Buffer is smaller than size of file %s, %d>%d\n", 
					ifilename, size, buffersize);
			return 0;
		}
	}

	int readbytes = unzReadCurrentFile(m_hFile, *ibuffer,size);
	if(readbytes != size)
		ComPrintf("CZipFile::OpenFile: Warning, only read %d of %d. %s , %s\n", 
		readbytes, size,ifilename, m_archiveName);

	unzCloseCurrentFile(m_hFile);
	return size;
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


#define CENTRAL_HDR_SIG	'\001','\002'	/* the infamous "PK" signature bytes, */
#define LOCAL_HDR_SIG	'\003','\004'	/*  sans "PK" (so unzip executable not */
#define END_CENTRAL_SIG	'\005','\006'	/*  mistaken for zipfile itself) */
#define EXTD_LOCAL_SIG	'\007','\010'	/* [ASCII "\113" == EBCDIC "\080" ??] */

#define DEF_WBITS	15	/* Default LZ77 window size */

#define CRCVAL_INITIAL  0L

typedef struct ZIP_local_file_header_s {
	unsigned char version_needed_to_extract[2];
	unsigned short general_purpose_bit_flag;
	unsigned short compression_method;
	unsigned short last_mod_file_time;
	unsigned short last_mod_file_date;
	unsigned long crc32;
	unsigned long csize;
	unsigned long ucsize;
	unsigned short filename_length;
	unsigned short extra_field_length;
} ZIP_local_file_header;

typedef struct ZIP_central_directory_file_header_s {
	unsigned char version_made_by[2];
	unsigned char version_needed_to_extract[2];
	unsigned short general_purpose_bit_flag;
	unsigned short compression_method;
	unsigned short last_mod_file_time;
	unsigned short last_mod_file_date;
	unsigned long crc32;
	unsigned long csize;
	unsigned long ucsize;
	unsigned short filename_length;
	unsigned short extra_field_length;
	unsigned short file_comment_length;
	unsigned short disk_number_start;
	unsigned short internal_file_attributes;
	unsigned long external_file_attributes;
	unsigned long relative_offset_local_header;
} ZIP_central_directory_file_header;

typedef struct ZIP_end_central_dir_record_s {
	unsigned short number_this_disk;
	unsigned short num_disk_start_cdir;
	unsigned short num_entries_centrl_dir_ths_disk;
	unsigned short total_entries_central_dir;
	unsigned long size_central_directory;
	unsigned long offset_start_central_directory;
	unsigned short zipfile_comment_length;
} ZIP_end_central_dir_record;

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






#define FILE_ONDISK  0xfffe

static const char hdr_central[4] = { 'P', 'K', CENTRAL_HDR_SIG };
static const char hdr_local[4] = { 'P', 'K', LOCAL_HDR_SIG };
static const char hdr_endcentral[4] = { 'P', 'K', END_CENTRAL_SIG };
static const char hdr_extlocal[4] = { 'P', 'K', EXTD_LOCAL_SIG };

/*--------------------------------------------------------------------------*/

/* *INDENT-OFF* */

static void load_ecdr(ZIP_end_central_dir_record * ecdr, const unsigned char *buff)
{
	ecdr->number_this_disk 					= LittleShort(*(short *)(buff + E_NUMBER_THIS_DISK));
	ecdr->num_disk_start_cdir				= LittleShort(*(short *)(buff + E_NUM_DISK_WITH_START_CENTRAL_DIR));
	ecdr->num_entries_centrl_dir_ths_disk	= LittleShort(*(short *)(buff + E_NUM_ENTRIES_CENTRL_DIR_THS_DISK));
	ecdr->total_entries_central_dir			= LittleShort(*(short *)(buff + E_TOTAL_ENTRIES_CENTRAL_DIR));
	ecdr->size_central_directory			= LittleLong(*(long *)(buff + E_SIZE_CENTRAL_DIRECTORY));
	ecdr->offset_start_central_directory	= LittleLong(*(long *)(buff + E_OFFSET_START_CENTRAL_DIRECTORY));
	ecdr->zipfile_comment_length			= LittleShort(*(short *)(buff + E_ZIPFILE_COMMENT_LENGTH));
}

static gboolean read_cdfh(ZIP_central_directory_file_header * cdfh, file_t infile)
{
	byte buff[ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE];

	if (FileRead(buff, ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE, infile) < ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE) return (gfalse);

	cdfh->version_made_by[0]			= buff[C_VERSION_MADE_BY_0];
	cdfh->version_made_by[1]			= buff[C_VERSION_MADE_BY_1];
	cdfh->version_needed_to_extract[0]	= buff[C_VERSION_NEEDED_TO_EXTRACT_0];
	cdfh->version_needed_to_extract[1]	= buff[C_VERSION_NEEDED_TO_EXTRACT_1];
	cdfh->general_purpose_bit_flag		= LittleShort(*(short *)(buff + C_GENERAL_PURPOSE_BIT_FLAG));
	cdfh->compression_method			= LittleShort(*(short *)(buff + C_COMPRESSION_METHOD));
	cdfh->last_mod_file_time			= LittleShort(*(short *)(buff + C_LAST_MOD_FILE_TIME));
	cdfh->last_mod_file_date			= LittleShort(*(short *)(buff + C_LAST_MOD_FILE_DATE));
	cdfh->crc32							= LittleLong(*(long *)(buff + C_CRC32));
	cdfh->csize							= LittleLong(*(long *)(buff + C_COMPRESSED_SIZE));
	cdfh->ucsize						= LittleLong(*(long *)(buff + C_UNCOMPRESSED_SIZE));
	cdfh->filename_length				= LittleShort(*(short *)(buff + C_FILENAME_LENGTH));
	cdfh->extra_field_length			= LittleShort(*(short *)(buff + C_EXTRA_FIELD_LENGTH));
	cdfh->file_comment_length			= LittleShort(*(short *)(buff + C_FILE_COMMENT_LENGTH));
	cdfh->disk_number_start				= LittleShort(*(short *)(buff + C_DISK_NUMBER_START));
	cdfh->internal_file_attributes		= LittleShort(*(short *)(buff + C_INTERNAL_FILE_ATTRIBUTES));
	cdfh->external_file_attributes		= LittleLong(*(long *)(buff + C_EXTERNAL_FILE_ATTRIBUTES));
	cdfh->relative_offset_local_header	= LittleLong(*(long *)(buff + C_RELATIVE_OFFSET_LOCAL_HEADER));

	return (gtrue);
}

static gboolean read_lfh(ZIP_local_file_header * lfh, file_t infile)
{
	byte buff[ZIP_LOCAL_FILE_HEADER_SIZE];

	if (FileRead(buff, ZIP_LOCAL_FILE_HEADER_SIZE, infile) < ZIP_LOCAL_FILE_HEADER_SIZE) return (gfalse);

	lfh->version_needed_to_extract[0]	= buff[L_VERSION_NEEDED_TO_EXTRACT_0];
	lfh->version_needed_to_extract[1]	= buff[L_VERSION_NEEDED_TO_EXTRACT_1];
	lfh->general_purpose_bit_flag		= LittleShort(*(short *)(buff + L_GENERAL_PURPOSE_BIT_FLAG));
	lfh->compression_method				= LittleShort(*(short *)(buff + L_COMPRESSION_METHOD));
	lfh->last_mod_file_time				= LittleShort(*(short *)(buff + L_LAST_MOD_FILE_TIME));
	lfh->last_mod_file_date				= LittleShort(*(short *)(buff + L_LAST_MOD_FILE_DATE));
	lfh->crc32							= LittleLong(*(long *)(buff + L_CRC32));
	lfh->csize							= LittleLong(*(long *)(buff + L_COMPRESSED_SIZE));
	lfh->ucsize							= LittleLong(*(long *)(buff + L_UNCOMPRESSED_SIZE));
	lfh->filename_length				= LittleShort(*(short *)(buff + L_FILENAME_LENGTH));
	lfh->extra_field_length				= LittleShort(*(short *)(buff + L_EXTRA_FIELD_LENGTH));

	return (gtrue);
}

/* *INDENT-ON* */

static archive_entry_t *Archive_insert_entry( archive_t * ar, const char *name, const ZIP_central_directory_file_header * cdfh )
{
	archive_entry_t *cur;
	archive_entry_t *prev;
	archive_entry_t *next = NULL;
	archive_entry_t *ae = NULL;

	/* check if a file with the same name has already been added */
	cur = ar->first;
	prev = NULL;
	while( cur ) {
		if( !strcmp(cur->filename, name) ) {
			if( prev ) {
				ae = prev->next = (archive_entry_t *)SafeMalloc(sizeof(archive_entry_t));
			} else {
				ae = ar->first = (archive_entry_t *)SafeMalloc(sizeof(archive_entry_t));
			}

			next = cur->next;
			SafeFree( cur );

			break;
		}
		prev = cur;
		cur = cur->next;
	}

	if( !ae ) {
		/* add to the end of the list */
		if( ar->first ) {
			cur = ar->first;
			while( cur->next )
				cur = cur->next;
			ae = cur->next = (archive_entry_t *) SafeMalloc(sizeof(archive_entry_t));
		} else {
			ae = ar->first = (archive_entry_t *) SafeMalloc(sizeof(archive_entry_t));
		}
	}

	ae->archive		= ar;
	ae->filename 	= copystring( name );
	ae->next		= next;
	ae->buffer		= NULL;
	ae->buffer_pos	= 0;
	ae->info 		= (ZIP_central_directory_file_header *) SafeMalloc(sizeof(ZIP_central_directory_file_header));
	memcpy(ae->info, cdfh, sizeof(ZIP_central_directory_file_header));

	ar->num_entries++;
	return( ae );
}

static int Archive_read_zip_entries( archive_t * ar )
{
	size_t								cur_offs;
	size_t								new_offs;
	size_t								len;
	char								buff[1024];
	ZIP_central_directory_file_header	cdfh;
	ZIP_local_file_header				lfh;
	archive_entry_t						*curentry;

	cur_offs = 0;
	while ((FileRead(buff, sizeof(hdr_local), ar->file) >= sizeof(hdr_local)) && (!memcmp(buff, hdr_local, sizeof(hdr_local))) && (read_lfh(&lfh, ar->file))) {
		new_offs = cur_offs + sizeof(hdr_local) + ZIP_LOCAL_FILE_HEADER_SIZE + lfh.filename_length + lfh.extra_field_length + lfh.csize;
		if ((lfh.filename_length > sizeof(buff)) || (FileRead(buff, lfh.filename_length, ar->file) < lfh.filename_length)) return (0);	/* Broken zipfile? */
		buff[lfh.filename_length] = 0;

		if ((buff[lfh.filename_length - 1] != '/') && (buff[lfh.filename_length - 1] != PATH_SEPARATOR)) {
			/* partially convert lfh to cdfh */
			zeromem(&cdfh, sizeof(cdfh));
			cdfh.version_needed_to_extract[0] = lfh.version_needed_to_extract[0];
			cdfh.version_needed_to_extract[1] = lfh.version_needed_to_extract[1];
			cdfh.general_purpose_bit_flag = lfh.general_purpose_bit_flag;
			cdfh.compression_method = lfh.compression_method;
			cdfh.last_mod_file_time = lfh.last_mod_file_time;
			cdfh.last_mod_file_date = lfh.last_mod_file_date;
			cdfh.crc32 = lfh.crc32;
			cdfh.csize = lfh.csize;
			cdfh.ucsize = lfh.ucsize;
			cdfh.relative_offset_local_header = cur_offs;

			curentry = Archive_insert_entry( ar, buff, &cdfh );

			if( FileSeek( ar->file, lfh.extra_field_length, FS_CURRENT ) )
				return( 0 );	/* broken zipfile */
		}
		if( FileSeek( ar->file, cur_offs = new_offs, FS_BEGIN ) )
			return( 0 );	/* broken zipfile */
	}

	if( !cur_offs ) {	/* no headers found */
		/* treat this file as a plain uncompressed file */

		zeromem(&cdfh, sizeof(cdfh));
		cdfh.version_made_by[0] = 0x16;	/* Zip version 2.2 rev 6 ??? */
		cdfh.version_made_by[1] = 0x06;
		cdfh.version_needed_to_extract[0] = 20;	/* Unzip version 2.0 rev 0 */
		cdfh.version_needed_to_extract[1] = 00;
		cdfh.compression_method = FILE_ONDISK;

		len = FileLength( ar->file );
		cdfh.csize = len;
		cdfh.ucsize = len;
		cdfh.relative_offset_local_header = 0;
		Archive_insert_entry( ar, buff, &cdfh );
	}

	return (1);
}

static gboolean Archive_read_zip_directory(archive_t * ar)
{
	archive_entry_t						*curentry;
	ZIP_end_central_dir_record			ecdr;
	ZIP_central_directory_file_header	cdfh;
	byte								buff[1024];	/* read ZIPfile from end in 1K chunks */
	size_t								cur_offs;
	size_t								min_offs;
	size_t								central_directory_offset;
	const size_t						step = ZIP_END_CENTRAL_DIR_RECORD_SIZE + sizeof(hdr_endcentral);
	unsigned int						search_pos;
	register byte						*search_ptr;

	if( !ar->file )
		return( gfalse );
	if( FS_FAILED(FileSeek(ar->file, 0, FS_END)))
		return( gfalse );

	cur_offs = FileTell( ar->file );
	if( (long)cur_offs == -1 )
		return( gfalse );

	if( cur_offs >= (65535 + ZIP_END_CENTRAL_DIR_RECORD_SIZE + sizeof(hdr_endcentral)) ) {
		min_offs = cur_offs - (65535 + ZIP_END_CENTRAL_DIR_RECORD_SIZE + sizeof(hdr_endcentral));
	} else {
		min_offs = 0;
	}

	/* Try to find ZIPfile central directory structure */
	/* For this we have to search from end of file the signature "PK" */
	/* after which follows a two-byte END_CENTRAL_SIG */
	while (cur_offs > min_offs) {

		if (cur_offs >= sizeof(buff) - step) {
			cur_offs -= sizeof(buff) - step;
		} else {
			cur_offs = 0;
		}

		FileSeek( ar->file, cur_offs, FS_BEGIN );
		search_pos = FileRead( buff, sizeof(buff), ar->file );

		if( search_pos >= step ) {
			for( search_ptr = &buff[search_pos - step]; search_ptr > buff; search_ptr--)
				if ((*search_ptr == 'P') && (!memcmp(search_ptr, hdr_endcentral, sizeof(hdr_endcentral)))) {
					/* central directory structure found */
					central_directory_offset = cur_offs + (unsigned long) search_ptr - (unsigned long) buff;
					load_ecdr( &ecdr, &search_ptr[sizeof(hdr_endcentral)] );
					if (FS_FAILED(FileSeek(ar->file, central_directory_offset + sizeof(hdr_endcentral) + ZIP_END_CENTRAL_DIR_RECORD_SIZE, FS_BEGIN))
					    || FS_FAILED(FileSeek(ar->file, ecdr.zipfile_comment_length, FS_CURRENT))
					    || FS_FAILED(FileSeek(ar->file, ecdr.offset_start_central_directory, FS_BEGIN))) goto rebuild_cdr;	/* broken central directory */

					/* now read central directory structure */
					for (;;) {
						if( (FileRead(buff, sizeof(hdr_central), ar->file) < sizeof(hdr_central)) || (memcmp(buff, hdr_central, sizeof(hdr_central))) ) {
							if( ar->first ) {
								return( gtrue );		/* finished reading central directory */
							} else {
								goto rebuild_cdr;	/* broken central directory */
							}
						}
						if( (!read_cdfh(&cdfh, ar->file)) || (cdfh.filename_length > sizeof(buff)) || (FileRead(buff, cdfh.filename_length, ar->file) < cdfh.filename_length) ) {
							return (gfalse);	/* broken zipfile? */
						}
						buff[cdfh.filename_length] = 0;

						if( (buff[cdfh.filename_length - 1] != '/') && (buff[cdfh.filename_length - 1] != PATH_SEPARATOR) ) {
							curentry = Archive_insert_entry( ar, (char *)buff, &cdfh );
						}
						if( FS_FAILED(FileSeek(ar->file, cdfh.extra_field_length + cdfh.file_comment_length, FS_CURRENT)) ) {
							return( gfalse );	/* broken zipfile? */
						}
					}
				}
		}
	}

rebuild_cdr:
	/* If we are here, we did not succeeded to read central directory */
	/* If so, we have to rebuild it by reading each ZIPfile member separately */
	if( FS_FAILED( FileSeek( ar->file, 0, FS_BEGIN ) ) )
		return( gfalse );
	if( !Archive_read_zip_entries(ar) )
		return( gfalse );

	return( gtrue );
}

/* PUBLIC FUNCTIONS */

gboolean Archive_Load(archive_t *ar, const char *filename)
{
	ar->first = NULL;
	ar->num_entries = 0;

	ar->file = FileOpen( filename, OPEN_READONLY );
	if( !ar->file )
		return( gfalse );

	if( !Archive_read_zip_directory(ar) )
		return( gfalse );

	return (1);
}

void Archive_Free( archive_t *ar )
{
	archive_entry_t *cur;
	archive_entry_t *next;

	for (cur = ar->first; cur; cur = next) {
		next = cur->next;

		SafeFree(cur->info);
		SafeFree(cur->filename);
		SafeFree(cur->buffer);
		cur->buffer_pos = 0;
	}

	if (ar->file) {
		FileClose(ar->file);
		ar->file = NULL;
	}
}

gboolean Archive_FileExists( const archive_t *ar, const char *name, size_t *size )
{
	archive_entry_t *f = Archive_FindName(ar, name);

	if (!f)
		return (gfalse);
	if (size)
		*size = f->info->ucsize;

	return (gtrue);
}

gboolean Archive_Read( byte *buf, int buflen, const archive_entry_t *f )
{
	size_t					bytes_left;
	size_t					size;
	byte					buff[1024];
	int						err;
	ZIP_local_file_header	lfh;
	z_stream				zs;
	file_t					infile;

	infile = f->archive->file;

	if( f->info->compression_method == FILE_ONDISK ) {
		size = min(buflen, f->info->ucsize);
		FileRead(buf, size, infile);
	} else {

		if ((FS_FAILED(FileSeek(infile, f->info->relative_offset_local_header, FS_BEGIN)))
			|| (FileRead(buff, sizeof(hdr_local), infile) < sizeof(hdr_local))
		    || (memcmp(buff, hdr_local, sizeof(hdr_local))) || (!read_lfh(&lfh, infile))
			|| (FS_FAILED(FileSeek(infile, lfh.filename_length + lfh.extra_field_length, FS_CURRENT)))) {
			return (gfalse);
		}

		switch( f->info->compression_method ) {
		case 0:
			size = min( buflen, f->info->ucsize );
			FileRead( buf, size, infile );
			break;
		case Z_DEFLATED:
			bytes_left = f->info->csize;
			zs.next_out = (Byte *) buf;
			zs.avail_out = buflen;
			zs.zalloc = (alloc_func) 0;
			zs.zfree = (free_func) 0;

			/* undocumented: if wbits is negative, zlib skips header check */
			err = inflateInit2(&zs, -DEF_WBITS);
			if (err != Z_OK) return (gfalse);

			while (bytes_left) {
				zs.next_in = (Byte *) buff;
				if (bytes_left > sizeof(buff))
					size = sizeof(buff);
				else
					size = bytes_left;
				zs.avail_in = FileRead(buff, size, infile);

				err = inflate(&zs, bytes_left > size ? Z_PARTIAL_FLUSH : Z_FINISH);
				bytes_left -= size;
			}
			inflateEnd(&zs);

			if ((err != Z_STREAM_END) && ((err != Z_BUF_ERROR) || zs.avail_out)) {
				return (gfalse);
			}

			/* do crc check */
			if( buflen == f->info->ucsize ) {
				if( crc32( CRCVAL_INITIAL, buf, buflen ) != f->info->crc32 ) {
					return (gfalse);
				}
			}
			break;
		default:	/* unknown compression algorithm */
			return (gfalse);
		}
	}

	return( gtrue );
}

archive_entry_t *Archive_FindName( const archive_t * ar, const char *name )
{
	archive_entry_t *f;

	f = ar->first;
	while (f) {
#if CASESENSITIVE
		if (!strcmp(f->filename, name)) {
#else
		if (!G_stricmp(f->filename, name)) {
#endif
			return (f);
		}
		f = f->next;
	}

	return (NULL);
}

int Archive_GetFileList( const archive_t * ar, const char *path, const char *extension, char *listbuf, int bufsize )
{
	archive_entry_t	*f;
	unsigned int	path_len;
	unsigned int	ext_len;
	unsigned int	len;
	int				num = 0;
	char			*listptr;
	char			filename[MAX_OSPATH];
	char			filepath[MAX_OSPATH];

	if(!ar || !path || !extension || !listbuf || !bufsize)
		return (0);

	path_len = strlen( path );
	ext_len = strlen( extension );

	listptr = listbuf;

	for( f = ar->first; f; f = f->next ) {
		len = strlen( f->filename );

		if(len < (path_len + ext_len + 1)) continue;

		/* check path */
		COM_ExtractFilePath( f->filename, filepath );

		/* remove the slash */
		filepath[strlen(filepath)-1] = 0;
#if CASESENSITIVE
		if( strcmp(filepath, path) ) continue;
#else
		if( G_stricmp(filepath, path) ) continue;
#endif

		/* check extension */
#if CASESENSITIVE
		if( G_stricmp(extension, f->filename + len - ext_len) ) continue;
#else
		if( strcmp(extension, f->filename + len - ext_len) ) continue;
#endif

		COM_ExtractFileName( f->filename, filename );
		len = strlen( filename );

		if(len > bufsize) continue;

		/* add to list */
		G_strncpyz( listptr, filename, bufsize );
		bufsize -= (len+1);
		listptr += (len+1);
		num++;
	}

	return( num );
}



















#endif







