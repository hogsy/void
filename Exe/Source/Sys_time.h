#ifndef VOID_SYS_TIME
#define VOID_SYS_TIME


class CTime
{
public:
	CTime();
	~CTime();

	bool Init();
	void Update();
	void Reset();

private:

	float GetElapsedTime();
		
	float			m_fbaseTime;
	float			m_flastTime;

	_int64			m_dticks_per_sec;
	_int64			m_dPerformanceTimerStart;
	float			m_fsecs_per_tick;
	bool			m_bPerformanceTimerEnabled;
	unsigned long	m_ulMMTimerStart;

};

#endif