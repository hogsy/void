#ifndef INC_VOID_EXPORT
#define INC_VOID_EXPORT

#include "I_console.h"
#include "I_mem.h"

/*
==========================================
Exe Exports
==========================================
*/

struct I_Void
{
	I_Void()  { console = 0; memManager = 0; }
	virtual ~I_Void() { console = 0; memManager = 0; }

	virtual float & GetCurTime()=0;
	virtual float & GetFrameTime()=0;
	virtual const char * GetCurPath()=0;

	//Interfaces
	I_Console     * console;
	I_MemManager  * memManager;
};


#endif