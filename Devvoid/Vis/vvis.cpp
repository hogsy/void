
/*
							vvis
	This program calculates the pvs for each node in a level.
	A set bit in the vis info means that that leaf _is_ visible.

*/



#include "../Std_lib.h"
#include "vis.h"


/*
============
main
============
*/

void CompileVis(const char * szPath, const char * szFileName)
{
	ComPrintf("\n\n======== vVis ========\n");

	vis_run(szPath, szFileName);
}


