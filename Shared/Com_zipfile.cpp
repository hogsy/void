#include "Com_zipfile.h"
#include "Com_defs.h"

//App print function
void ComPrintf(char *string,...);

/*
=======================================================================
Struct Definations

Thanks to Samuel H. Smith for Format Info
=======================================================================
*/

#define LOCAL_FILE_HEADER_SIGNATURE		0x04034b50L
#define CENTRAL_FILE_HEADER_SIGNATURE	0x02014b50L
#define END_CENTRAL_DIR_SIGNATURE		0x06054b50L

/*typedef struct {
	unsigned int version_needed_to_extract;
    unsigned int general_purpose_bit_flag;
	unsigned int compression_method;
	unsigned int last_mod_file_time;
	unsigned int last_mod_file_date;
	unsigned long crc32;
	unsigned long compressed_size;
    unsigned long uncompressed_size;
	unsigned int filename_length;
	unsigned int extra_field_length;
}ZipHeader_t;
*/

typedef struct                                   // local file header
    {
    unsigned long lf_sig;                               // signature (0x04034b50)
    unsigned int  lf_extract,                           // vers needed to extract
          lf_flag,                              // general purpose flag
          lf_cm,                                // compression method
          lf_time,                              // file time
          lf_date;                              // ..and file date
    unsigned long lf_crc,                               // CRC-32 for file
          lf_csize,                             // compressed size
          lf_size;                              // uncompressed size
    int   lf_fn_len,                            // file name length
          lf_ef_len;                            // extra field length
    }ZipHeader_t;


typedef struct {
	word version_made_by;
	word version_needed_to_extract;
	word general_purpose_bit_flag;
	word compression_method;
	word last_mod_file_time;
	word last_mod_file_date;
	long crc32;
	long compressed_size;
	long uncompressed_size;
	word filename_length;
	word extra_field_length;
	word file_comment_length;
	word disk_number_start;
	word internal_file_attributes;
	long external_file_attributes;
	long relative_offset_local_header;
}ZipCentralDirHeader_t;


typedef struct {
	word number_this_disk;
	word number_disk_with_start_central_directory;
	word total_entries_central_dir_on_this_disk;
	word total_entries_central_dir;
	long size_central_directory;
	long offset_start_central_directory;
	word zipfile_comment_length;
}ZipCentralDirFoot_t;

/*
=======================================================================
CZipFile Implementation
=======================================================================
*/


CZipFile::CZipFile()
{
	m_fp =0;
}


CZipFile::~CZipFile()
{
	if(m_fp)
		fclose(m_fp);
}



//CArchive Implementation
bool  CZipFile::Init(const char * base, const char * archive)
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

	ZipHeader_t ziphead;
	unsigned long sig=0;

/*	if (fread(&sig, sizeof(long),1,m_fp))
	{
		switch(sig)
		{
			case LOCAL_FILE_HEADER_SIGNATURE:
				ComPrintf("LOCAL FILE HEADER\n");
				break;
			case CENTRAL_FILE_HEADER_SIGNATURE:
				ComPrintf("CENTRAL FILE\n");
				break;
			case END_CENTRAL_DIR_SIGNATURE:
				ComPrintf("Central END\n");
			break;

		}
	}
*/
	fread(&ziphead,sizeof(ZipHeader_t),1,m_fp);

	sig = ziphead.lf_sig;

		switch(sig)
		{
			case LOCAL_FILE_HEADER_SIGNATURE:
				ComPrintf("LOCAL FILE HEADER\n");
				break;
			case CENTRAL_FILE_HEADER_SIGNATURE:
				ComPrintf("CENTRAL FILE\n");
				break;
			case END_CENTRAL_DIR_SIGNATURE:
				ComPrintf("Central END\n");
			break;

		}

	ziphead;
	return false;
}


long  CZipFile::OpenFile(const char* path, byte ** buffer)
{
	return 0;
}

void  CZipFile::ListFiles()
{
}

bool  CZipFile::GetFileList (CStringList * list)
{
	return false;
}
