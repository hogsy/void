#ifndef __VOID_MUSIC__
#define __VOID_MUSIC__


class CMusic
{
public:
	CMusic();
	~CMusic();
	
	bool Init(); 
	bool Shutdown();
};

extern CMusic * g_pMusic;

#endif