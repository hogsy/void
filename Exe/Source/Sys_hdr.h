#ifndef VOID_STANDARD_HEADER
#define VOID_STANDARD_HEADER

#define WIN32_LEAN_AND_MEAN

#define _VOID_EXE_ 1				// used by shared files

//disable warnings
//#pragma warning(disable : 4018)     // signed/unsigned mismatch
//#pragma warning(disable : 4305)		// truncation from const double to float

#include <winsock2.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <winuser.h>
#include <objbase.h>
#include <mmsystem.h>
#include <direct.h>

#include "Com_defs.h"
#include "Com_mem.h"

#include "I_filesystem.h"

#include "Sys_cons.h"
#include "Util_sys.h"
#include "World.h"

#define INITGUID

#define PROTOCOL_VERSION	1

enum eGameState
{
	INCONSOLE,
	INMENU,
	INGAMECONSOLE,
	INGAME
};

extern HWND			g_hWnd;
extern HINSTANCE	g_hInst;
extern char			g_exedir[COM_MAXPATH];
extern char			g_gamedir[COM_MAXPATH];

extern float		g_fframeTime;
extern float		g_fcurTime;

extern eGameState	g_gameState;

#endif
