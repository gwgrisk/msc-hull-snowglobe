
#include "stdafx.h"
#include "AppLog.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>

#include <GL\GL.h>

namespace AntiMatter
{
	AppLog::AppLog() : 
		m_sLogFile		( "C:\\app-events.log" ),
		m_bInitialized	( false )
	{
		// standard ctor required for singleton instantiation
		m_bInitialized = Initialize();
	}
	AppLog::~AppLog()
	{		
		Uninitialize();
	}
		
	bool AppLog::Initialize()
	{
		// ensure the requested path is valid
		// attempt to open file for writing
		// initialize critical section

		if( m_bInitialized )
			return true;

		using namespace std;

		string	sPath;
		int		nPos = m_sLogFile.find_last_of(string("\\"));

		sPath.resize( nPos + 1 );
		std::copy( m_sLogFile.begin(), m_sLogFile.begin() + nPos + 1, sPath.begin() );

		if( Shell::PathExists(sPath) )
			m_Stream.open( m_sLogFile.c_str(), std::ios::out );

		if( m_Stream.good() )
			InitializeCriticalSection(&m_csCritSec);

		return m_Stream.good() ? true:false;
	}
	void AppLog::Uninitialize()
	{
		if( m_Stream.good() )
			m_Stream.close();

		DeleteCriticalSection( &m_csCritSec );
	}

	void AppLog::LogMsg( const std::string & sMessage, BOOL bPrependDateTime )
	{
		// using sprintf() since boost::format not allowed semester one
		// http://www.boost.org/libs/format/index.html

		if( ! m_bInitialized )
			return;						

		EnterCriticalSection(&m_csCritSec);

			if( bPrependDateTime )
			{
				char		sFormat[100];
				SYSTEMTIME	LocalTime;

				GetLocalTime( &LocalTime );

				sprintf_s(
					sFormat,
					"[%.2d.%.2d.%.4d %.2d:%.2d:%.2d]%c",
					LocalTime.wDay,
					LocalTime.wMonth,
					LocalTime.wYear,
					LocalTime.wHour, 
					LocalTime.wMinute, 
					LocalTime.wSecond,
					'\0'
				);

				m_Stream << std::string(sFormat).c_str();
			}
				
			m_Stream << sMessage.c_str() << std::endl;

			OutputDebugStringA( sMessage.c_str() );

		LeaveCriticalSection( &m_csCritSec );
	}	
	void AppLog::LogMsg( const char* psFmtMessage, ... )
	{
		// algorithm src for vararg parsing
		// http://c-faq.com/~scs/cclass/int/sx11b.html

		// using sprintf() since boost::format not allowed semester one
		// http://www.boost.org/libs/format/index.html

		if( ! m_bInitialized )
			return;	

		using namespace std;

		EnterCriticalSection(&m_csCritSec);

			int			nFmtMessageLen	= strlen( psFmtMessage );
			int			nFmtPos;
			const char* pFmt;
			char*		pStr;
			va_list		argp;
			int			i		= 0;
			int			nIntVal;		
			char		sOutput[2048];		// final message string can be max size of 2k

			va_start( argp, psFmtMessage );

			for( nFmtPos = 0, pFmt = psFmtMessage; nFmtPos < nFmtMessageLen; nFmtPos ++, pFmt ++ )
			{
				if( *pFmt != '%' )
				{
					memcpy( &sOutput[i++], pFmt, 1 );
					continue;
				}

				switch( *++pFmt )
				{
					case 'c':
						nIntVal = va_arg(argp, int);
						memcpy( &sOutput[i++], pFmt, 1 );					
						break;

					case 'd':
						nIntVal = va_arg(argp, int);
						pStr	= _itoa( nIntVal, &sOutput[i], 10 );
					
						while(*pStr != '\0')
						{
							pStr ++;
							i ++;
						}

						break;

					case 's':
						pStr = va_arg(argp, char *);

						if( pStr )
						{
							memcpy(&sOutput[i], pStr, strlen(pStr));
							i+= strlen(pStr);
						}
						break;

					case 'x':
						nIntVal = va_arg(argp, int);
						pStr	= _itoa( nIntVal, &sOutput[i], 16 );

						// advance pStr until we hit a \0
						while(*pStr != '\0')
						{
							pStr ++;
							i ++;
						}
						break;

					case '%':
						memcpy(&sOutput[i++], ++pFmt, 1);
						break;
				}
			}									

			sOutput[i++] = '\0';

			char		sFormat[50];
			SYSTEMTIME	LocalTime;

			GetLocalTime( &LocalTime );

			sprintf_s(
				sFormat,
				"[%.2d.%.2d.%.4d %.2d:%.2d:%.2d]%c",
				LocalTime.wDay,
				LocalTime.wMonth,
				LocalTime.wYear,
				LocalTime.wHour, 
				LocalTime.wMinute, 
				LocalTime.wSecond,
				'\0'
			);

			m_Stream << string(sFormat).c_str();				
			m_Stream << string(sOutput).c_str() << std::endl;

			string sOutputThis = string(sOutput) + string("\r\n");

			OutputDebugStringA( sOutputThis.c_str() );

		LeaveCriticalSection( &m_csCritSec );

	}
		
	void AppLog::OutputGlErrors()
	{
		// this is just a debug function really, apply liberally to buggy glCode to determine
		// where you've messed up
		using namespace std;
		
		GLenum err;

        while ( (err = glGetError()) != GL_NO_ERROR )
		{
			stringstream	ssError;
			string			sError;

			switch( err )
			{
			case GL_INVALID_ENUM:
				ssError << "GLError: Invalid enum 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;

			case GL_INVALID_VALUE:
				ssError << "GLError: Invalid value 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;

			case GL_INVALID_OPERATION:
				ssError << "GLError: Invalid operation 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;

			case GL_OUT_OF_MEMORY:
				ssError << "GLError: Out of memory 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;

			case 0x506:		// GL_INVALID_FRAMEBUFFER_OPERATION (not mentioned in gl.h on windows platform)
				ssError << "GLError: Invalid Framebuffer Operation 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;

			case GL_STACK_OVERFLOW:
				ssError << "GLError: Stack overflow 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;

			case GL_STACK_UNDERFLOW:
				ssError << "GLError: Unknown glError code 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;

			default:
				ssError << "GLError: Unknown glError code 0x" << std::hex << err << endl;
				sError = ssError.str();
				break;
			}

			OutputDebugStringA( sError.c_str() );
		}
			
	}
};
