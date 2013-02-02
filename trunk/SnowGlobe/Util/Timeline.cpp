
#include "stdafx.h"
#include "Timeline.h"

namespace AntiMatter
{
	Timeline::Timeline() : 
		m_nSecondsPerTick	(0.0),
		m_nDeltaTime		(-1.0),
		m_TimelineStart		(0),
		m_PausedTime		(0),
		m_PreviousTime		(0),
		m_CurrentTime		(0),
		m_StopTime			(0),
		m_bStopped			(false)
	{
		// timer frequency tells us how many ticks per second the performance counter returns
		__int64 countsPerSecond;
		QueryPerformanceFrequency( (LARGE_INTEGER*) &countsPerSecond );
		this->m_nSecondsPerTick = 1.0 / (double)countsPerSecond;
	}
	
	void Timeline::Start()
	{
		__int64 startTime;
		QueryPerformanceCounter( (LARGE_INTEGER*) &startTime );

		if( m_bStopped )
		{
			m_PausedTime	+= (startTime - m_StopTime);
			m_PreviousTime	= startTime;
			m_StopTime		= 0;
			m_bStopped		= false;
		}
	}
	void Timeline::Tick()
	{
		// read the current time and compute the delta time from the previous frame

		if( m_bStopped )
		{
			m_nDeltaTime = 0.0;
			return;
		}
		
		__int64 timeNow;
		QueryPerformanceCounter( (LARGE_INTEGER*) &timeNow );
		m_CurrentTime = timeNow;

		m_nDeltaTime	= (m_CurrentTime - m_PreviousTime) * m_nSecondsPerTick;
		m_PreviousTime	= m_CurrentTime;

		if(m_nDeltaTime < 0.0)
			m_nDeltaTime = 0.0;
	}	
	void Timeline::Stop()
	{
		if( ! m_bStopped )
		{
			__int64 timeNow;
			QueryPerformanceCounter((LARGE_INTEGER*) &timeNow );
			m_StopTime	= timeNow;
			m_bStopped	= true;
		}
	}
	void Timeline::Reset()
	{
		__int64 timeNow;
		QueryPerformanceCounter( (LARGE_INTEGER*) &timeNow );

		m_TimelineStart = timeNow;
		m_PreviousTime	= timeNow;
		m_StopTime		= 0;
		m_bStopped		= false;
	}

	double Timeline::DeltaTime() 
	{
		return (double)m_nDeltaTime; 
	}
	double Timeline::SimTime()
	{
		double rSimTime;

		if( m_bStopped )		
			rSimTime = (double)( (m_StopTime - m_TimelineStart) * m_nSecondsPerTick );		
		else		
			rSimTime = (double)( ((m_CurrentTime - m_PausedTime)- m_TimelineStart) * m_nSecondsPerTick );		

		return rSimTime;
	}
};