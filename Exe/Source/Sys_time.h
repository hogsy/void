#ifndef VOID_SYS_TIME
#define VOID_SYS_TIME

#include "Sys_hdr.h"

class CTime
{
public:
	CTime();
	~CTime();

	bool Init();
	
	void Update()
	{
		g_fcurTime =  (this->*GetTime)() - m_fBaseTime;
		g_fframeTime = g_fcurTime - m_fLastTime;
		m_fLastTime = g_fcurTime;
	}
	void Reset();

private:

	float	m_fBaseTime;
	float	m_fLastTime;
	float	m_fSecsPerTick;
	_int64	m_dTimerStart;

	//UpdateFunc GetTime;
	float	(CTime::*GetTime)();

	//One of these gets bound to the GetTime func pointer
	float	GetPerformanceCounterTime();
	float	GetMMTime();
};

#endif