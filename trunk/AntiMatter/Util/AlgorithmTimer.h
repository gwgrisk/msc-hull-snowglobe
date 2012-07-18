
#pragma once

#include <WTypes.h>

namespace AntiMatter
{
namespace Algorithm
{
	class AlgorithmTimer
	{
	private:
		LARGE_INTEGER m_TickFrequency;
		LARGE_INTEGER m_StartTime;
		LARGE_INTEGER m_StopTime;

	public:
	
		AlgorithmTimer()
		{		
			QueryPerformanceFrequency(&m_TickFrequency);

			m_StartTime.QuadPart	= 0;
			m_StopTime.QuadPart		= 0;
		}	

		void Start()
		{
			QueryPerformanceCounter(&m_StartTime);
		}
		void Stop()
		{
			QueryPerformanceCounter(&m_StopTime);
		}
		void Reset()
		{			
			m_StartTime.QuadPart	= 0;
			m_StopTime.QuadPart		= 0;
		}

		double DurationSecs()
		{
			return (double)(( (double)m_StopTime.QuadPart - (double)m_StartTime.QuadPart ) / (double)m_TickFrequency.QuadPart );
		}
	};
};
};
