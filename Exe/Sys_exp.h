#ifndef VOID_EXPORTS_IMPLEMENTATION
#define VOID_EXPORTS_IMPLEMENTATION

#include "I_void.h"
#include "Sys_hdr.h"

struct VoidExport : public I_Void
{
	float GetCurTime()	 { return System::GetCurTime();		}
	float GetFrameTime() { return System::GetFrameTime();		}
	const char * GetCurPath(){ return System::GetCurGamePath();	}

	void SystemError(const char *message) { System::FatalError(message); };
};


#endif