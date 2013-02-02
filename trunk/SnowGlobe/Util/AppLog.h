#pragma once

#include <stdio.h>
#include "Singleton.h"
#include "ShellUtils.h"
#include <fstream>
#include <string>

namespace AntiMatter 
{
	class AppLog : public Singleton<AppLog>
	{
	private:
		bool				m_bInitialized;
		CRITICAL_SECTION	m_csCritSec;
		std::string			m_sLogFile;
		std::ofstream		m_Stream;

	private:
		bool Initialize();
		void Uninitialize();

	public:
		AppLog();
		~AppLog();

		void LogMsg( const std::string & sMessage, BOOL bPrependDateTime = TRUE );
		void LogMsg( const char* psFmtMessage, ... );

		void OutputGlErrors();
	};
};

