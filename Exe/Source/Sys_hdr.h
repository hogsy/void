#ifndef VOID_STANDARD_HEADER
#define VOID_STANDARD_HEADER

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <memory.h>
#include <winuser.h>
#include <objbase.h>
#include <mmsystem.h>
#include <direct.h>

  #define __VOIDALPHA	1
//#define INCLUDE_MUSIC	1

#include "Com_defs.h"
#include "Com_util.h"

#include "I_mem.h"
#include "I_console.h"
#include "I_file.h"
#include "I_filesystem.h"

#include "World.h"

//======================================================================================
//======================================================================================

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
	//Current Time
	extern float	g_fframeTime;
	extern float	g_fcurTime;

	//Common System functions
	I_Console *	GetConsole();
	const char* GetExePath();
	const char* GetCurrentPath();
	eGameState  GetGameState();
	void SetGameState(eGameState state);

	//Windows specefic
	HINSTANCE	GetHInstance();
	HWND		GetHwnd();
}

#endif