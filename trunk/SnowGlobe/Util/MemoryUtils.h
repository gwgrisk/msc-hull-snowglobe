

#pragma once
#include <windows.h>

namespace AntiMatter
{
	namespace Memory
	{
		HRESULT QueryFreePhysicalMemory( __int64 * pFreeMegaBytes );
		HRESULT QueryPhysicalMemoryUsedPercent( int * pnUsedMemory );
	};
};
