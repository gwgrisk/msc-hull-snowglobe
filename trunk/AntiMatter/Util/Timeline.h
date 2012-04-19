
#pragma once

#include <Windows.h>

namespace AntiMatter
{
	class Timeline
	{
	private:
		__int64 	m_TimelineStart;
		__int64 	m_PreviousTime;
		__int64 	m_CurrentTime;
		__int64 	m_StopTime;
		__int64 	m_PausedTime;

		double		m_nSecondsPerTick;
		double		m_nDeltaTime;

		bool		m_bStopped;

	public:
		Timeline();
	
		void Start();
		void Tick();
		void Stop();
		void Reset();

		double DeltaTime();
		double SimTime();
	};
};
