#ifndef VOID_STANDARD_HEADER
#define VOID_STANDARD_HEADER

#define WIN32_LEAN_AND_MEAN

#include "Com_defs.h"
#include "Com_mem.h"
#include "Com_parms.h"

#include "I_hunkmem.h"
#include "I_console.h"
#include "I_file.h"

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

struct I_FileReader;
struct I_InputFocusManager;

namespace System
{
	//Common System functions
	
	//Current Time
	float GetCurTime();
	float GetFrameTime();
	
	I_Console  *	GetConsole();
	const char *	GetExePath();
	const char *	GetCurGamePath();
	eGameState		GetGameState();

	I_FileReader *  CreateFileReader(EFileMode mode);
	I_InputFocusManager * GetInputFocusManager();

	void  FatalError(const char *error);
	void  SetGameState(eGameState state);
	
	//Windows specefic
	HINSTANCE		GetHInstance();
	HWND			GetHwnd();
}
#endif