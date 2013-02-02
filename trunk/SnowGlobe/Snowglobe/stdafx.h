
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <atlbase.h>
#include <atlstr.h>

#pragma comment(lib, "opengl32.lib")

#define TIXML_USE_TICPP
#if defined _DEBUG
  #pragma comment(lib, "ticppd.lib")
#else
  #pragma comment(lib, "ticpp.lib")
#endif

#pragma comment(lib, "util.lib")

#include <AntiMatter\Exceptions.h>
#include <AntiMatter\AppLog.h>


/*
// Can't use any of this ( thanks gxbase :( )
// -- leak detection ----------------------------------------------------------
#ifdef _DEBUG	
	// http://msdn.microsoft.com/en-us/library/e5ewb1h3(v=VS.80).aspx
	#define _CRTDBG_MAP_ALLOC	
	#include <stdlib.h>
	#include <crtdbg.h>
	// #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	// #define new DEBUG_NEW
#endif


#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		//_CrtSetBreakAlloc( n );
#endif	

#ifdef _DEBUG
		_CrtDumpMemoryLeaks();
#endif

*/