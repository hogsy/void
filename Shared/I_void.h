#ifndef INC_VOID_EXPORT
#define INC_VOID_EXPORT

#include "I_console.h"

/*
==========================================
This is what the exe exports
==========================================
*/
typedef struct VoidExport_s
{
	 char * basedir;
	 char * gamedir;

	 float* curtime;
	 float* frametime;

	 //Add interfaces here
	 I_ExeConsole * vconsole;

}VoidExport_t;


#endif