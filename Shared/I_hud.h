#ifndef INC_INTERFACE_HUD
#define INC_INTERFACE_HUD


//Need to be able to pass some sort of layout info to the hud
//i.e pass image/model index, screenposition, link it with an auto updated number/string


enum EHudItemType
{
	HUDTEXT,		//passing char data that should be allocated and displayed
	HUDSTRING,		//passing a pointer to a static string
	HUDINT,
	HUDFLOAT,
	HUDIMAGE,
	HUDMODEL
};


#define HUD_DEFAULTMSGTIME	3.0
	
/*
==========================================
Renderer HUD Interface
==========================================
*/
struct I_RHud
{
	//Printing
	virtual void __stdcall HudPrintf(int x, int y, float time,char *msg,...) =0;
	virtual void __stdcall HudPrint(char *msg, int x, int y, float time =0.0, int color=0) =0;
	virtual void __stdcall PrintMessage(char *msg, int color=0, float time=HUD_DEFAULTMSGTIME) =0;
	
//	virtual int  pascal AddHudItem(void *data, EHudItemType type, int x, int y, float time) = 0;
//	virtual int  pascal AddHudGfx(int index, EHudItemType type, int x, int y, float time) = 0;
//	virtual int  pascal AddHudMsg(char *data,  int x, int y, float time = HUD_DEFAULTMSGTIME) = 0;
//	virtual bool pascal RemoveHudItem(int index) = 0;

	//Add image drawing here
};


#endif	