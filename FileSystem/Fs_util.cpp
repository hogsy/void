#include "Fs_hdr.h"
#include "Fs_filesys.h"
#include "Com_util.h"

extern CFileSystem * g_pFileSystem;

namespace FileUtil
{

FILESYSTEM_API bool FindFileExtension(char * ext, int extlen, const char * path)
{
	char filename[COM_MAXPATH];
	if(g_pFileSystem->FindFileName(filename,COM_MAXPATH,path))
	{
		Util::ParseExtension(ext,extlen,filename);
		return true;
	}
	return false;
}


}