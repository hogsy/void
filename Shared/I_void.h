#ifndef INC_VOID_EXPORT
#define INC_VOID_EXPORT

#include "I_console.h"

/*
==========================================
This is what the exe exports
==========================================
*/
struct VoidExport_t
{
	VoidExport_t(const char * pbasedir,
				 const char * pgamedir,
				 float * icurtime,
				 float * iframetime) : basedir(pbasedir), gamedir(pgamedir),
									   curtime(icurtime), frametime(iframetime)
	{}

	~VoidExport_t()
	{
		basedir = 0;
		gamedir = 0;
		curtime = 0;
		frametime = 0;
		vconsole = 0;
	}
	
	//FIXME, remove these, delegate to filesystem
	const char * basedir;
	const char * gamedir;

	float* curtime;
	float* frametime;

	//Add interfaces here
	I_ExeConsole * vconsole;
};


#endif