#ifndef VOID_STANDARD_HEADER
#define VOID_STANDARD_HEADER

#define WIN32_LEAN_AND_MEAN

#define __VOIDALPHA	1

#include "Com_defs.h"
#include "Com_mem.h"

#include "I_hunkmem.h"
#include "I_console.h"

#include "Com_cvar.h"

//======================================================================================
//======================================================================================
const char VOID_DEFAULTMAPEXT  []	= "wld";
const char VOID_DEFAULTGAMEDIR []	= "Game";

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
	//Common System functions
	
	//Current Time
	const float &	GetCurTime();
	const float &	GetFrameTime();
	
	I_Console  *	GetConsole();
	const char *	GetExePath();
	const char *	GetCurrentPath();
	eGameState		GetGameState();

	void  FatalError(const char *error);
	void  SetGameState(eGameState state);
	
	//Windows specefic
	HINSTANCE		GetHInstance();
	HWND			GetHwnd();
}
#endif