#ifndef VOID_MISC_UTILITY
#define VOID_MISC_UTILITY

#include "Com_defs.h"

//======================================================================================
//======================================================================================

namespace Util
{

void ParseExtension(char *ext, int bufsize, const char *filename);
void RemoveExtension (char *out, int bufsize, const char *in);
void ParseFilePath(char *path, int pathlen, const char *file);
void SetDefaultExtension (char *filename, const char *extension);
bool CompareExts(const char *file, const char *ext);		

void ShowMessageBox(const char * str, const char *title=0);

//Move these to error handler
void HRPrint(HRESULT hr, const char* str);
void HRShowMessageBox(HRESULT hr, const char* str);

//FIXME ! need to write a proper CParms class to handle all this
int	 BufParse(const char *string, char ** szargv);
}

#endif