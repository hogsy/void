#ifndef VOID_SYS_TIME
#define VOID_SYS_TIME

#include "Sys_hdr.h"
#include <mmsystem.h>

class CTime
{
public:
	CTime();
	~CTime();

	bool Init();
	
	void Update()
	{
		System::g_fcurTime =  GetTime() - m_fBaseTime;
		System::g_fframeTime = System::g_fcurTime - m_fLastTime;
		m_fLastTime = System::g_fcurTime;
	}
	void Reset();

private:

	float	m_fBaseTime;
	float	m_fLastTime;

	//UpdateFunc GetTime;
	float	(*GetTime)();

	static float	m_fSecsPerTick;
	static _int64	m_dTimerStart;

	//One of these gets bound to the GetTime func pointer
	static float	GetPerformanceCounterTime();
	static float	GetMMTime();
};

#endif