#ifndef VOID_SYS_TIME
#define VOID_SYS_TIME

/*
====================================================
A Simple Timer class which tries to use the 
Win32 HighPerformance Timer. Will default to 
the Windows Multimedia timer if the High Performance
Timer is not found.

I call the GetCurrent/Frame time functions from a 
pair of globally accessible functions which I define 
in namespace System{} to hide the implementation of 
CTime from all the other game code.
====================================================
*/
class CTime
{
public:

	CTime();
	~CTime();

	void Init();
	void Reset();
	
	//Call this onces per game frame
	void Update();

	//Access funcs
	inline const float & GetCurrentTime() const { return m_fCurrentTime; }
	inline const float & GetFrameTime()   const { return m_fFrameTime;   } 

private:

	typedef float (CTime::* TimeUpdateFunc) ();
	TimeUpdateFunc m_pfnUpdate;

	float	m_fBaseTime;
	float	m_fLastTime;
	float	m_fSecsPerTick;

	float	m_fCurrentTime;
	float	m_fFrameTime;

	//QueryPerformancesCounter related 64bit integers.
	//These are Microsoft specific, and will have to be changed for
	//different compilers.
	_int64	m_dTimerStart;
	_int64  m_dCurTime;

	//One of these gets bound to the TimeUpdateFunc pointer
	float	GetPerformanceCounterTime();
	float	GetMMTime();
};

#endif