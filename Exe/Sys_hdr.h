#ifndef VOID_STANDARD_HEADER
#define VOID_STANDARD_HEADER

#define WIN32_LEAN_AND_MEAN

#define __VOIDALPHA	1

#pragma warning(disable : 4786)

#include "Com_mem.h"
#include "Com_defs.h"

#include "I_hunkmem.h"
#include "I_console.h"

#include "Com_cvar.h"

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
	I_Console  *	GetConsole();
	const char *	GetExePath();
	const char *	GetCurrentPath();
	eGameState		GetGameState();
	
	void SetGameState(eGameState state);

	//Windows specefic
	HINSTANCE	GetHInstance();
	HWND		GetHwnd();
}

#endif