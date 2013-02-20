
#pragma once

#include <string>
#include <algorithm>
#include <sstream>


class AppException : public std::runtime_error
{
private:
	std::string		m_sMessage;
	std::string		m_sLocation;
	DWORD			m_dwSystemError;
	DWORD			m_dwDomainError;

public:
	AppException( const std::string& sMessage, DWORD dwDomainError, DWORD dwSystemError, const char* file, long line );
	AppException( const std::string& sMessage, DWORD dwDomainError = 0, DWORD dwSystemError = 0 );

	virtual const char* what() throw();
	
	DWORD GetSystemError() const { return m_dwSystemError; };
	DWORD GetDomainError() const { return m_dwDomainError; };

private:
	void FormatLocation( const char* file, long line, std::string & sLocation );
	void FormatErrorMsg( const std::string & sMessage, DWORD dwError, const std::string & sError, const std::string & sLocation, std::string & sFormatted );
	void FormatFullMsg( const std::string & sMessage, DWORD dwSysErr, const std::string & sLocation, std::string & sFormatted );
};
