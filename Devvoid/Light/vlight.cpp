#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "../Std_lib.h"
#include "light.h"



//==========================================================================
//==========================================================================

void CompileLightmaps(const char * szPath, const char * szFileName)
{
	ComPrintf("======== vLight ========\n");

	CWorld * pWorld = CWorld::CreateWorld(szFileName);
	if(!pWorld)
	{
		ComPrintf("Unable to open world file: %s\n", szFileName);
		return;
	}

	pWorld->DestroyLightData();
		
	if (light_run(pWorld))
		light_write(szPath);
		
	if (pWorld)
		CWorld::DestroyWorld(pWorld);
}



