#ifndef INC_VOID_EXPORT
#define INC_VOID_EXPORT

#include "I_console.h"
#include "I_hunkmem.h"

/*
==========================================
Exe Exports
==========================================
*/

struct I_Void
{
	I_Void()  { console = 0; hunkManager = 0; }
	virtual ~I_Void() { console = 0; hunkManager = 0; }

	virtual float GetCurTime()=0;
	virtual float GetFrameTime()=0;
	virtual const char * GetCurPath()=0;

	//Tell the Main application that the given module died so that
	//it can cleanly shutdown everything.
	virtual void SystemError(const char *message)=0;

	//Interfaces
	I_Console     * console;
	I_HunkManager * hunkManager;
};


#endif