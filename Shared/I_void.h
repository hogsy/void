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
	VoidExport_t(float * icurtime,
				 float * iframetime) :  curtime(icurtime), frametime(iframetime)
	{}

	~VoidExport_t()
	{
		curtime = 0;
		frametime = 0;
		vconsole = 0;
	}
	
	float* curtime;
	float* frametime;

	//Add interfaces here
	I_ExeConsole * vconsole;
};


#endif