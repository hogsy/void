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

void   ShowMessageBox(const char * str, const char *title=0);

//Move these to error handler
void   HRPrint(HRESULT hr, const char* str);
void   HRShowMessageBox(HRESULT hr, const char* str);

//FIXME ! need to write a proper CParms class to handle all this
int	   BufParse(const char *string, char ** szargv);
}

#endif