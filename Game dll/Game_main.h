#ifndef VOID_GAME_MAIN
#define VOID_GAME_MAIN

#include "I_console.h"

class CBuffer;
struct I_World;

/*
======================================
Implementation of the Game Interface
======================================
*/
class CGame : public I_Game,
			  public I_ConHandler
{
public:

	CGame();
	~CGame();

	bool InitGame();
	void ShutdownGame();
	int  GetVersion(){ return GAME_VERSION; }

	bool LoadWorld(I_World * pWorld);
	void UnloadWorld();

	void RunFrame(float curTime);

	bool SpawnEntity(CBuffer &buf);

	//Client funcs
	bool ClientConnect(int clNum, CBuffer &userInfo, bool reconnect);
	void ClientUpdateUserInfo(int clNum, CBuffer &userInfo);
	void ClientBegin(int clNum);
	void ClientDisconnect(int clNum);
	void ClientCommand(int clNum, CBuffer &command);

	//Console Handler
	void HandleCommand(HCMD cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);


private:

	I_World * m_pWorld;
	void InitializeVars();

};


#endif