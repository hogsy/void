#ifndef _UTIL_SYS
#define _UTIL_SYS

/*
Other utility funcs
*/


void __cdecl InitMemReporting(void);
void __cdecl EndMemReporting(void);

void	Util_GetExtension(const char *filename, char *ext);
void	Util_RemoveExtension(const char *in, char *out);
void	Util_FindExtension(const char*filename, char *out);
void	Util_GetFilePath(const char *file, char *path);
void	Util_DefaultExtension (char *path, const char *extension);
void	Util_ErrorMessage(HRESULT hr, const char* str);
void	Util_ErrorMessageBox(HRESULT hr, const char* str);

#endif