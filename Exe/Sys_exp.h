#ifndef VOID_EXPORTS_IMPLEMENTATION
#define VOID_EXPORTS_IMPLEMENTATION

#include "I_void.h"
#include "Sys_hdr.h"

struct VoidExport : public I_Void
{
	float & GetCurTime()	 { return System::g_fcurTime;		}
	float & GetFrameTime()	 { return System::g_fframeTime;		}
	const char * GetCurPath(){ return System::GetCurrentPath();	}
};


#endif