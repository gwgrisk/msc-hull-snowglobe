
// We're locked into using GxBase::App for this assignment.
//
// since the gxbase::App object is static, it makes controlling object creation order 
// awkward.
//
// So, in order to ensure creation of AppLog object before the Simulation object comes
// into existance, and remains available until the Simulation object has been destroyed, 
// the simulation object must be allocated on the heap.
//
// This is an imposed design constraint.
//
// gxbase does in fact, make many things awkward.
// Memory leak detection with CRT debug heap functions is one of them
// http://msdn.microsoft.com/en-us/library/e5ewb1h3(v=VS.80).aspx
//
//

#include "stdafx.h"

#include <gxbase.h>
#include <AntiMatter\AppLog.h>

#include "Simulation.h"
#include "SeasonalTimeline.h"
#include "InputMgr.h"
#include "EffectMgr.h"
#include "slowglobe-cfg.h"


class GxApp :public gxbase::App
{
private:
	Simulation* m_pSim;

public:
	GxApp() :
		m_pSim	( NULL )
	{
		using namespace AntiMatter;

		// -- Initialize the singletons
		AppLog::CreateInstance();
		if( ! AppLog::Incarnated() )
		{
			ATLTRACE( "failed to create the AppLog, halting execution\r\n" );
			return;
		}
		
		SeasonalTimeline::CreateInstance();
		if( ! SeasonalTimeline::Incarnated() )
		{
			AppLog::Ref().LogMsg( "%s failed to create the SeasonalTimeline, halting execution", __FUNCTION__ );
			return;
		}
		SeasonalTimeline::Ref() = SeasonalTimeline( g_Cfg.SeasonCfg() );


		// Initialize the InputMgr singleton
		InputMgr::CreateInstance();
		if( ! InputMgr::Incarnated() )
		{
			AppLog::Ref().LogMsg( "%s failed to create the InputMgr, halting execution", __FUNCTION__ );
			return;
		}				

		AppLog::Ref().LogMsg("Attempting Simulation start..");

		m_pSim = new (std::nothrow) Simulation();
		if( ! m_pSim )
		{
			AppLog::Ref().LogMsg( "%s failed to allocate heap for the simulation, halting execution\r\n", __FUNCTION__ );
			return;
		}		
	}
	~GxApp()
	{	
		try
		{
			using namespace AntiMatter;

			if( m_pSim )
			{
				delete m_pSim;
				m_pSim = NULL;
			}

			SeasonalTimeline::DestroyInstance();
			InputMgr::DestroyInstance();

			if( EffectMgr::Incarnated() )
				EffectMgr::DestroyInstance();

			AppLog::Ref().LogMsg("Exiting GxApp()");
			AppLog::DestroyInstance();
		}
		catch(...)
		{
		}
	}
};


static GxApp leia;