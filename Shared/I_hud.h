#ifndef INC_HUD_INTERFACE
#define INC_HUD_INTERFACE


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


const float HUD_DEFAULTMSGTIME = 3.0f;


struct I_HudRenderer
{
	virtual void Printf(int x, int y, float time, const char *msg, ...)=0;

	virtual void AddConMessage(const char *msg, int color=0, float time=HUD_DEFAULTMSGTIME) =0;

	//Print a console type message on the top of the screen
//	virtual void SetConMessageOrientation();
	

};


#endif	