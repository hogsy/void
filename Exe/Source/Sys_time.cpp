#include "Sys_hdr.h"
#include "Sys_time.h"


float	g_fframeTime=0;
float	g_fcurTime=0;


/*
=====================================
Constructor
=====================================
*/
CTime::CTime()
{
	m_fbaseTime = 0.0f;
	m_flastTime = 0.0f;
	m_fsecs_per_tick = 0.0f;

	m_dticks_per_sec = 	0;
	m_dPerformanceTimerStart= 0;
	
	m_bPerformanceTimerEnabled =false;
	m_ulMMTimerStart = 0;
}


/*
=====================================
Destructor
=====================================
*/
CTime::~CTime()
{
}


/*
=====================================
Initialize the timer
=====================================
*/
bool CTime::Init()
{
	if (!QueryPerformanceFrequency((LARGE_INTEGER *)&m_dticks_per_sec))
	{ 
		// no performance counter available
		m_ulMMTimerStart = timeGetTime();
		m_fsecs_per_tick = 1.0f/1000.0f;
		m_bPerformanceTimerEnabled = false;
	}
	else
	{ 
		// performance counter is available, use it instead of multimedia timer
		QueryPerformanceCounter((LARGE_INTEGER *)&m_dPerformanceTimerStart);
		m_fsecs_per_tick = (float)(1.0)/((float)m_dticks_per_sec);
		m_bPerformanceTimerEnabled = true;
	}
	return true;
}



/*
=====================================
Update
=====================================
*/
void CTime::Update()
{
	g_fcurTime = GetElapsedTime() - m_fbaseTime;
	g_fframeTime = g_fcurTime - m_flastTime;
	m_flastTime = g_fcurTime;
}

/*
=====================================

=====================================
*/
void CTime::Reset()
{
	m_fbaseTime = GetElapsedTime();
	m_flastTime = g_fcurTime = g_fframeTime = 0.0;
}

/*
=====================================

=====================================
*/
float CTime::GetElapsedTime()
{
	if (m_bPerformanceTimerEnabled)
	{
		static _int64 curtime;
		QueryPerformanceCounter((LARGE_INTEGER *)&curtime);
		return(((float)(curtime-m_dPerformanceTimerStart)) * m_fsecs_per_tick);
	}
	else
	{
		return(((float)(timeGetTime()- m_ulMMTimerStart)) * m_fsecs_per_tick);
	}
}


