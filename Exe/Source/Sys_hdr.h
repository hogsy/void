#ifndef VOID_STANDARD_HEADER
#define VOID_STANDARD_HEADER

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <winuser.h>
#include <objbase.h>
#include <mmsystem.h>
#include <direct.h>


#define _VOID_EXE_ 1				// used by shared files

#define __VOIDALPHA	1

#define INCLUDE_MUSIC	1


#include "Com_defs.h"
#include "Com_mem.h"

#include "I_console.h"
#include "I_filesystem.h"

#include "Util_sys.h"
#include "World.h"

#define INITGUID


enum eGameState
{
	INCONSOLE,
	INMENU,
	INGAMECONSOLE,
	INGAME
};

I_Console *	Sys_GetConsole();

extern HWND			g_hWnd;
extern HINSTANCE	g_hInst;
extern char			g_exedir[COM_MAXPATH];

//Current Time
extern float		g_fframeTime;
extern float		g_fcurTime;

//Current Game State
extern eGameState	g_gameState;

#endif
