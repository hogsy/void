#include "Sys_time.h"


float	g_fframeTime=0;		//The Global Frame Time
float	g_fcurTime=0;		//The Global Current Time 


/*
=====================================
Constructor
=====================================
*/
CTime::CTime()
{
	m_fBaseTime = 0.0f;
	m_fLastTime = 0.0f;
	m_fSecsPerTick = 0.0f;

	m_dTimerStart= 0;
	
	GetTime = 0;
}


/*
=====================================
Destructor
=====================================
*/
CTime::~CTime()
{
	GetTime = 0;
}


/*
=====================================
Initialize the timer
=====================================
*/
bool CTime::Init()
{
	_int64	dTicksPerSec=0;

	//Use MM time if NOT able to use the High Frequency timer
	if (!QueryPerformanceFrequency((LARGE_INTEGER *)&dTicksPerSec))
	{ 
		// no performance counter available
		m_dTimerStart = timeGetTime();
		m_fSecsPerTick = 1.0f/1000.0f;
		GetTime = &CTime::GetMMTime;
	}
	else
	{ 
		// performance counter is available, use it instead of multimedia timer
		QueryPerformanceCounter((LARGE_INTEGER *)&m_dTimerStart);
		m_fSecsPerTick = (float)(1.0)/((float)dTicksPerSec);
		GetTime = &CTime::GetPerformanceCounterTime;
	}
	return true;
}


/*
=====================================

=====================================
*/
void CTime::Reset()
{
	m_fBaseTime =  (this->*GetTime)();
	m_fLastTime = g_fcurTime = g_fframeTime = 0.0;
}

/*
=====================================

=====================================
*/

float CTime::GetPerformanceCounterTime()
{
	static _int64 curTime=0;
	QueryPerformanceCounter((LARGE_INTEGER *)&curTime);
	return(((float)(curTime-m_dTimerStart)) * m_fSecsPerTick);
}

float CTime::GetMMTime()
{
	return(((float)(timeGetTime()- m_dTimerStart)) * m_fSecsPerTick);
}
