#ifndef VOID_GAME_MAIN
#define VOID_GAME_MAIN

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

	void RunFrame();

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