
#include "stdafx.h"
#include "ShellUtils.h"

#include <string>
#include <sstream>
#include <algorithm>

#include <atlstr.h>

#include <shlobj.h>
#include <shlguid.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <Shellapi.h>

namespace AntiMatter
{
namespace Shell
{
	HRESULT CreateFolder( std::string & sPath )
	{
		// create the destination folder if it doesn't already exist

		HRESULT			hr		= S_OK;
		DWORD			dwError = 0;
		std::wstring	wsPath;

		if( sPath.length() == 0 || sPath.length() < 4 )
			return E_UNEXPECTED;

		std::copy( sPath.begin(), sPath.end(), wsPath.begin() );
		
		if( GetFileAttributes( wsPath.c_str() ) == INVALID_FILE_ATTRIBUTES )
		{
			dwError = SHCreateDirectoryEx( NULL, wsPath.c_str(), NULL );

			if ( (dwError != ERROR_SUCCESS) && (dwError != ERROR_FILE_EXISTS) && (dwError != ERROR_ALREADY_EXISTS) )
				hr = HRESULT_FROM_WIN32(dwError);
		}

		return hr;
	}
	HRESULT DetermineDiskSpace( __int64* pllMegaBytesFree, const std::string & sPath )
	{
		// http://msdn.microsoft.com/en-us/library/aa364937(v=vs.85).aspx

		using namespace std;

		HRESULT 		hr				= S_OK;			   
		
		WCHAR			szRoot[4];
		LPCWSTR			szDiskRoot;				
		std::wstring	wsUNC;

		ULARGE_INTEGER	llFree, llTotal, llTotalFree;
		ULONG			lOneMegaByte	= 1048576;

		std::wstring	wsPath;
		std::copy( sPath.begin(), sPath.end(), wsPath.begin() );

		try
		{
			*pllMegaBytesFree = 0;

			if( sPath.length() == 0 || sPath.length() < 4 )
				throw (E_UNEXPECTED);

			std::copy( sPath.begin(), sPath.end(), wsPath.begin() );			

			// -- Get the root
			if( wsPath[1] == L':' )				// drive path
			{
				lstrcpyn((LPWSTR)szRoot, wsPath.c_str(), 4);
				szRoot[3] = 0;
			}
			else if ( wsPath[0] == L'\\' )		// UNC path
			{
				if ( wsPath[wsPath.length() - 1] != L'\\' )
				{
					wostringstream	os( wsPath );
					os << "\\";

					wsUNC = os.str();
				}				
				else
				{
					wsUNC.resize( wsPath.length() );
					std::copy( wsPath.begin(), wsPath.end(), wsUNC.begin() );					
				}
			}
			else
				throw( E_FAIL );
					
			szDiskRoot = wsUNC.length() ? wsUNC.c_str() : (LPCWSTR)szRoot;

			if( ! GetDiskFreeSpaceExW( szDiskRoot, &llFree, &llTotal, &llTotalFree) )
				throw ( E_FAIL );

			*pllMegaBytesFree = static_cast<__int64>  ( llFree.QuadPart / lOneMegaByte );

		}
		catch( HRESULT hrErr )
		{
			hr = hrErr;
		}
		catch(...)
		{
			ATLTRACE( _T("Unhandled exception in AntiMatter::Shell::DetermineDiskSpace()\r\n") );
		}

		return hr;
	}
	
	bool FileExists( const std::string & sFile )
	{
		// returns true if file exists		
		using namespace std;

		if( sFile.length() < 4 )
			return false;

		wstring wsFile;

		wsFile.resize( sFile.length() + 1 );
		std::copy( sFile.begin(), sFile.end(), wsFile.begin() );

		return ( GetFileAttributes( wsFile.c_str() ) == INVALID_FILE_ATTRIBUTES ? false : true );			
	}
	bool PathExists( const std::string & sPath )
	{
		// returns true if path (e.g. folder) exists

		SHFILEINFOA	shFileInfo;
		bool		bExists = false;

		ZeroMemory(&shFileInfo, sizeof(shFileInfo)); 

		if ( SHGetFileInfoA((LPCSTR)sPath.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_TYPENAME) != 0 )
		{
			if ( (strcmp(shFileInfo.szTypeName, "File Folder") == 0) || (strcmp(shFileInfo.szTypeName, "Local Disk") == 0) )
				bExists = true;
		}

		return bExists;
	}
	HRESULT FileHasExtension( bool & bHasExtension, const std::string & sFile, const std::string & sExtension )
	{
		using namespace std;

		string sExn;
		
		bHasExtension = false;

		if( ! FileExists(sFile) )	
			return E_INVALIDARG;		

		if( sExtension.length() == 0 || sExtension.length() > sFile.length() )
			return E_INVALIDARG;				

		sExn.resize( sExtension.length() );
		std::copy( sFile.end() - sExtension.length() , sFile.end(), sExn.begin() );

		if( sExn.compare(sExtension) == 0 )
			bHasExtension = true;

		return S_OK;
	}
	DWORD FileSize( const std::string & sFile )
	{
		// http://msdn.microsoft.com/en-us/library/windows/desktop/aa364955(v=vs.85).aspx

		if( sFile.length() < 4 )
			return 0;

		using namespace std;

		HANDLE	hFile		= INVALID_HANDLE_VALUE;
		DWORD	dwFileSize	= 0;
		wstring wsFileName;

		wsFileName.resize( sFile.length() );
		std::copy( sFile.begin(), sFile.end(), wsFileName.begin() );

		hFile = CreateFile( 
			wsFileName.c_str(), 
			GENERIC_READ, 
			0, 
			NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL, 
			NULL 
		);

		if( hFile == INVALID_HANDLE_VALUE )
			return 0;

		dwFileSize = GetFileSize( hFile, NULL );
		CloseHandle(hFile);

		if( dwFileSize == INVALID_FILE_SIZE )
			dwFileSize = 0;

		return dwFileSize;
	}
		
	HRESULT GetModuleFilePath( std::string & sFilePath )
	{
		// Get the full file path and name of the current module's .exe
		// trim off the module's filename from the path
		CString sPath;
		int		nBufSize	= MAX_PATH;
		DWORD	dwRes		= 0;

		USES_CONVERSION;
	
		dwRes = GetModuleFileName(NULL, sPath.GetBuffer(nBufSize), nBufSize);
		sPath.ReleaseBuffer();

		if (!dwRes)
			return HRESULT_FROM_WIN32(GetLastError());
	
		int nPos = sPath.ReverseFind(_T('\\'));
		if (nPos == -1)
			return E_UNEXPECTED;
	
		sFilePath = T2A( sPath.Left(nPos + 1).GetString() );

		return S_OK;
	}
};
};
