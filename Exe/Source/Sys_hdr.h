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

#define __VOIDALPHA	1
//#define INCLUDE_MUSIC	1

#include "Com_defs.h"
#include "Com_mem.h"

#include "I_console.h"
#include "I_filesystem.h"

#include "Util_sys.h"
#include "World.h"

#define INITGUID

//Game States
enum eGameState
{
	INCONSOLE,
	INMENU,
	INGAMECONSOLE,
	INGAME
};

namespace System
{

//Current Game State
extern eGameState	g_gameState;

//Current Time
extern float		g_fframeTime;
extern float		g_fcurTime;

//Common System functions
I_Console *	GetConsole();
HINSTANCE	GetHInstance();
HWND		GetHwnd();
const char* GetExeDir();

}

#endif