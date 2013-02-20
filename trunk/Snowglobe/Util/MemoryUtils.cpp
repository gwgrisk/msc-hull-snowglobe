
#include "stdafx.h"
#include "Memory.h"


namespace AntiMatter
{
namespace Memory
{
	HRESULT QueryFreePhysicalMemory( __int64 * pFreeMegaBytes )
	{
		// http://msdn.microsoft.com/en-us/library/aa366589(VS.85).aspx
		// return an indication of how much free memory is available (in mb)

		HRESULT			hr = E_FAIL;
		MEMORYSTATUSEX	MemoryStatus;

		ZeroMemory( &MemoryStatus, sizeof(MEMORYSTATUSEX) );
		*pFreeMegaBytes = 0;

		if( GlobalMemoryStatusEx(&MemoryStatus) )
		{
			*pFreeMegaBytes = ((MemoryStatus.ullAvailPhys / 1024) / 1024);
			hr				= S_OK;
		}

		return hr;
	}
	HRESULT QueryPhysicalMemoryUsedPercent( int * pnUsedMemory )
	{
		// http://msdn.microsoft.com/en-us/library/aa366589(VS.85).aspx
		// return system physical memory used as a percentage

		HRESULT				hr = E_FAIL;
		MEMORYSTATUSEX		MemStatusEx;

		MemStatusEx.dwLength = sizeof (MEMORYSTATUSEX);
		
		*pnUsedMemory = 0;

		if( GlobalMemoryStatusEx(&MemStatusEx) )
		{
			*pnUsedMemory	= MemStatusEx.dwMemoryLoad;
			hr				= S_OK;
		}
		else
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );
		}


		return hr;
	}
};
};
