#ifndef VOID_GAME_MAIN
#define VOID_GAME_MAIN

#include "Com_defs.h"
#include "Com_mem.h"
#include "Com_buffer.h"
#include "Com_vector.h"
#include "Net_defs.h"
#include "I_console.h"
#include "I_game.h"

/*
======================================
Implementation of the Game Interface
======================================
*/
class CGame : public I_Game
{
public:

	CGame();
	~CGame();

	bool InitGame();
	void ShutdownGame();
	int  GetVersion();

	void RunFrame(float curTime);

	bool SpawnEntity(CBuffer &buf);

	//Client funcs
	bool ClientConnect(int clNum, CBuffer &userInfo, bool reconnect);
	void ClientUpdateUserInfo(int clNum, CBuffer &userInfo);
	void ClientBegin(int clNum);
	void ClientDisconnect(int clNum);
	void ClientCommand(int clNum, CBuffer &command);

private:

};

//main game class
extern CGame * g_pGame;

//game imports
extern I_GameHandler * g_pImports;

#endif