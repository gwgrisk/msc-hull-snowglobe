
#pragma once

#include <string>


namespace AntiMatter
{
	namespace Shell
	{
		HRESULT CreateFolder( std::string & sPath );
		HRESULT DetermineDiskSpace( __int64* pllMegaBytesFree, const std::string & sPath );
	
		bool FileExists( const std::string & sFile );
		bool PathExists( const std::string & sPath );
		HRESULT FileHasExtension( bool & bHasExtension, const std::string & sFile, const std::string & sExtension );
		DWORD FileSize( const std::string & sFile );
	
		HRESULT GetModuleFilePath( std::string & sFilePath );
	};
};