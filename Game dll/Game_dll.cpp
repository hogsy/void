#include "Game_hdr.h"
#include "Game_main.h"


const char MEM_SZLOGFILE[] = "mem_game.log";

//Global Variables
I_GameHandler * g_pImports=0;
I_Console	  * g_pCons=0;
CGame		  *	g_pGame=0;

//==========================================================================
//==========================================================================


I_Game * GAME_GetAPI(I_GameHandler * pImports, I_Console * pConsole)
{
	g_pImports = pImports;
	g_pCons = pConsole;
	if(!g_pGame)
		g_pGame = new CGame();
	return g_pGame;
}

void GAME_Shutdown()
{
	if(g_pGame)
	{	delete g_pGame;
		g_pGame = 0;
	}
	g_pImports = 0;
	g_pCons = 0;
}

void ComPrintf(const char * text, ...)
{
	static char buffer[1024];
	
	va_list args;
	va_start(args, text);
	vsprintf(buffer, text, args);
	va_end(args);
	g_pCons->ComPrint(buffer);
}

int HandleOutOfMemory(size_t size)
{	g_pImports->FatalError("Game Dll is out of memory");
	return 0;
}
