#ifndef HUD_MAIN_H
#define HUD_MAIN_H

//Need to be able to pass some sort of layout info to the hud
//i.e pass image/model index, screenposition, link it with an auto updated number/string

#include "I_hud.h"


class CHudMessage;

/*
==========================================
Renderer HUD Interface
==========================================
*/
class CRHud : public I_HudRenderer
{
public:

	CRHud();
	~CRHud();

	//Draw all the items 
	void DrawHud();				

	//Printing
	void Printf(int x, int y, float time, const char *msg, ...);
	void AddConMessage(const char *msg, int color=0, float time=HUD_DEFAULTMSGTIME);

//	 int  __stdcall AddHudItem(void *data, EHudItemType type, int x, int y, float time);
//	 bool __stdcall RemoveHudItem(int index);
//	 int  __stdcall AddHudGfx(int index, EHudItemType type, int x, int y, float time) = 0;

	//Add image drawing here

private:
	CHudMessage	*   m_hmessages;//[MAX_MESSAGES];
	CHudMessage *   m_conmessages;//[MAX_TEXTLINES];		//text messages that appear on the top of the screen

	char m_hudBuffer[1024];
};


#endif



