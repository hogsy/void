#ifndef VOID_BOTHGAMES_DEFS_H
#define VOID_BOTHGAMES_DEFS_H

/*
============================================================================
This header contains the common code needed by
both the clientside DLL and serverside game DLL
hence: BothGames_defs.h
============================================================================
*/

#include "Net_defs.h"
#include "Net_protocol.h"


//Client sends this to the server as frequently as possible
struct ClCmd
{
	float	time;
	int		angles[3];
	short	forwardmove, 
			rightmove, 
			upmove;
};


#endif