#ifndef VOID_MISC_UTILITY
#define VOID_MISC_UTILITY

#include "Com_defs.h"

//======================================================================================
//======================================================================================

namespace Util
{

void   GetExtension(const char *filename, char *ext);
void   RemoveExtension(const char *in, char *out);
void   FindExtension(const char*filename, char *out);
void   GetFilePath(const char *file, char *path);
void   DefaultExtension (char *path, const char *extension);
void   ConfirmDir(char* dir);
void   PrintErrorMessage(HRESULT hr, const char* str);
void   ErrorMessageBox(HRESULT hr, const char* str);

}

#endif