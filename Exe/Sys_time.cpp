#include "Sys_hdr.h"
#include <mmsystem.h>
#include "Sys_time.h"

/*
=====================================
Constructor/Destructor
=====================================
*/
CTime::CTime() : 
		m_pfnUpdate(0),
		m_fBaseTime(0.0f),
		m_fLastTime(0.0f),
		m_fSecsPerTick(0.0f),
		m_fCurrentTime(0.0f),
		m_fFrameTime(0.0f),
		m_dTimerStart(0),
		m_dCurTime(0)
{
}

CTime::~CTime()
{	m_pfnUpdate = 0;
}

/*
=====================================
Initialize the timer
=====================================
*/
void CTime::Init()
{
	_int64	dTicksPerSec =0;

	if (QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&dTicksPerSec)))
	{ 
		// performance counter is available, use it instead of multimedia timer
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_dTimerStart));
		m_fSecsPerTick = (1.0f)/(static_cast<float>(dTicksPerSec));
		m_pfnUpdate = &CTime::GetPerformanceCounterTime;
	}
	else
	{ 
		//Use MM timer if unable to use the High Frequency timer
		m_dTimerStart = timeGetTime();
		m_fSecsPerTick = 1.0f/1000.0f;
		m_pfnUpdate = &CTime::GetMMTime;
	}
}

/*
================================================
Update time
================================================
*/
void CTime::Update()
{
	//The ->* operator binds the function pointer to the object pointed to by
	//the right hand pointer. which is THIS below
	m_fCurrentTime =  (this->*m_pfnUpdate)() - m_fBaseTime;
	m_fFrameTime = m_fCurrentTime - m_fLastTime;
	m_fLastTime = m_fCurrentTime;
}

/*
=====================================
Reset the Timer
=====================================
*/
void CTime::Reset()
{
	m_fBaseTime =  (this->*m_pfnUpdate)();
	m_fLastTime = m_fCurrentTime = m_fFrameTime = 0.0;
}

/*
=====================================
Time update funcs
=====================================
*/
float CTime::GetPerformanceCounterTime()
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_dCurTime));
	return (m_fSecsPerTick * (m_dCurTime - m_dTimerStart));
}

float CTime::GetMMTime()
{	return (m_fSecsPerTick * (timeGetTime()- m_dTimerStart));
}