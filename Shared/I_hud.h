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


#define HUD_DEFAULTMSGTIME	3.0
	

#endif	