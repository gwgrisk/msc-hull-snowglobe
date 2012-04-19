
#include "StdAfx.h"
#include "AppException.h"


AppException::AppException( const std::string& sMessage, DWORD dwDomainError, DWORD dwSystemError, const char* file, long line ) :
	std::runtime_error	( sMessage ),
	m_dwSystemError		( dwSystemError ),
	m_dwDomainError		( dwDomainError )
{
	FormatLocation( file, line, m_sLocation );
}

AppException::AppException( const std::string& sMessage, DWORD dwDomainError, DWORD dwSystemError )	: 
	std::runtime_error	( sMessage ),
	m_dwSystemError		( dwSystemError ),
	m_dwDomainError		( dwDomainError ),
	m_sLocation			( std::string("") )
{
}

const char* AppException::what() throw()
{
	//	Retrieve an error string suitable for logging for this exception
	FormatFullMsg( std::runtime_error::what(), m_dwSystemError, m_sLocation, m_sMessage );

	return m_sMessage.c_str();
}

void AppException::FormatLocation( const char* file, long line, std::string & sLocation )
{
	//	Formats location information for including in exception what() string
	using namespace std;

	ostringstream os;
	os << "line: " << "( " << line << " ) " << "in file: " << file;

	sLocation = os.str();
}
void AppException::FormatErrorMsg( const std::string & sMessage, DWORD dwError, const std::string & sError, const std::string & sLocation, std::string & sFormatted )
{
	//	Formats error string returned from exception
	std::ostringstream os;
	os << sMessage << " : " << std::hex << dwError << ":" << sError << sLocation;

	sFormatted = os.str();	
}	
void AppException::FormatFullMsg( const std::string & sMessage, DWORD dwSysErr, const std::string & sLocation, std::string & sFormatted )
{
	using namespace std;

	ostringstream	os;
	void*			pMessageBuffer	= NULL;			
	int				nSuccess		= 0;
		
	if ( dwSysErr != 0 )
	{
		os << "Sys error 0x" << std::hex << dwSysErr;
		
		if( dwSysErr != 1 )
		{			
			nSuccess = ::FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
				NULL, 
				dwSysErr,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
				(LPTSTR)&pMessageBuffer,
				0,
				NULL
			);

			if( nSuccess )
			{
				os << ": " << static_cast<const char*>(pMessageBuffer);
				::LocalFree( pMessageBuffer );
			}
		}		
	}

	os << " " << sLocation << " - " << sMessage;	
		
	sFormatted = os.str();
}
