#ifndef _INC_HUD
#define _INC_HUD

//#include "I_hud.h"
#include "I_clientRenderer.h"


//Need to be able to pass some sort of layout info to the hud
//i.e pass image/model index, screenposition, link it with an auto updated number/string

/*
Base Hud item class
Derive 
		image/model,			(get index of image or model and use that)	
		temp message,			(get message, format it, and store it for the duration it needs to be printed)
		string,int,float		(continously updated info, accept pointer to string,int,float etc, maybe add additional
								 color, effects, font info
displays from this
*/

#define MAX_TEXTLINES  6		//Max lines up at the top of the screen

#define MAX_MESSAGES  32		//Temporary messages that are stored in buffers
#define MAX_IMAGES	  16		//Images on screen
#define MAX_MODELS    16		//Models on screen
#define MAX_FIELDS	  32		//Includes constantly updated text, ints, floats for actual game hud

#define HUD_MSGBUFFER 256

/*
==========================================
Base Hud item class
==========================================
*/

class CHudItem
{
public:
	bool  inuse;
	float time;
	int	x;
	int	y;

	virtual void Draw() {};
	virtual void Reset();

	CHudItem();
	CHudItem(int ix, int iy, float itime);
	virtual ~CHudItem();
};

/*
==========================================
Message Class
==========================================
*/

class CHudMessage : public CHudItem
{
public:
	char *	data;//[HUD_MSGBUFFER];
	int		len;
	
	virtual void Draw();
	virtual void Reset();
	
	CHudMessage();
	CHudMessage(const char *msg, int ix, int iy, float itime);
	void Set(const char *msg, int ix, int iy, float itime);
	~CHudMessage();
};

/*
==========================================
Renderer HUD Interface
==========================================
*/
class CRHud
{
public:

	CRHud();
	~CRHud();

	void DrawHud();				//Draw all the items 

	//Printing
	void __stdcall HudPrintf(int x, int y, float time, char *msg);
	void __stdcall HudPrint(char *msg, int x, int y, float time =0.0, int color=0);
	void __stdcall PrintMessage(char *msg, int color=0, float time=HUD_DEFAULTMSGTIME);	//Console Type
	
//	 int  __stdcall AddHudItem(void *data, EHudItemType type, int x, int y, float time);
//	 bool __stdcall RemoveHudItem(int index);
//	 int  __stdcall AddHudGfx(int index, EHudItemType type, int x, int y, float time) = 0;

	//Add image drawing here


private:
	CHudMessage	*   m_hmessages;//[MAX_MESSAGES];
	CHudMessage  *  m_conmessages;//[MAX_TEXTLINES];		//text messages that appear on the top of the screen
};


#endif



