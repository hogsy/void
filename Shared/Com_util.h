#ifndef VOID_COM_UTILITY
#define VOID_COM_UTILITY

//======================================================================================
//======================================================================================

namespace Util {

void ParseExtension(char *ext, int bufsize, const char *filename);
void RemoveExtension (char *out, int bufsize, const char *in);
void ParseFilePath(char *path, int pathlen, const char *file);
void ParseFileName(char *name, int namelen, const char *path);
void SetDefaultExtension (char *filename, const char *extension);
bool CompareExts(const char *file, const char *ext);		

void ConfirmDir(char* dir);
bool PathExists(const char * path);

void ShowMessageBox(const char * str, const char *title=0);

//Move these to error handler

char * GetWin32ErrorMessage(ulong msgId, char *buf, int buflen);

void HRPrint(HRESULT hr, const char* str);
void HRShowMessageBox(HRESULT hr, const char* str);

}

#endif