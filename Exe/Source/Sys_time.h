#ifndef VOID_SYS_TIME
#define VOID_SYS_TIME


#include "Sys_hdr.h"


class CTime
{
public:
	CTime();
	~CTime();

	bool Init();
	
	inline void Update()
	{
		g_fcurTime = GetTime() - m_fBaseTime;
		g_fframeTime = g_fcurTime - m_fLastTime;
		m_fLastTime = g_fcurTime;
	}
	
	void Reset();

private:

	float	m_fBaseTime;
	float	m_fLastTime;

	float (*CTime::GetTime)();

	//One of these gets bound to the GetTime func pointer
	static float GetPerformanceCounterTime();
	static float GetMMTime();

	static float	m_fSecsPerTick;
	static _int64	m_dTimerStart;
};

#endif