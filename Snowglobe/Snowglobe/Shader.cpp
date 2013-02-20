
#include "stdafx.h"
#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <AntiMatter\ShellUtils.h>


Shader::Shader( GLint nProgramId, const std::string & sShaderFile, ShaderType sType ) :
	m_bInitialized	( false ),
	m_sShaderFile	( sShaderFile ),
	m_nShaderType	( sType ),	
	m_nShaderId		( 0 ),
	m_nProgramId	( nProgramId )
{
	glex::Load();

	if( nProgramId != 0 )	// if ProgramId == 0, allocation of ProgramId has failed
		Initialize();
}
Shader::Shader( const Shader & r ) :
	m_bInitialized	( &r != this ? r.m_bInitialized : m_bInitialized ),
	m_sShaderFile	( &r != this ? r.m_sShaderFile	: m_sShaderFile ),
	m_nShaderType	( &r != this ? r.m_nShaderType	: m_nShaderType ),
	m_nShaderId		( &r != this ? r.m_nShaderId	: m_nShaderId ),
	m_nProgramId	( &r != this ? r.m_nProgramId	: m_nProgramId )	
{
	glex::Load();	
}
Shader & Shader::operator=( const Shader & r )
{
	if( &r != this )
	{
		glex::Load();

		m_nShaderId		= r.m_nShaderId;
		m_bInitialized	= r.m_bInitialized;	
		m_nProgramId	= r.m_nProgramId;
		m_nShaderType	= r.m_nShaderType;
		m_sShaderFile	= r.m_sShaderFile;		
	}

	return *this;
}

Shader::~Shader()
{
	try
	{
		Uninitialize();
	}
	catch(...)
	{
		ATLTRACE(_T("Unanticipated exception caught in Shader() dtor\r\n"));
	}	
}

bool Shader::Initialize()
{	
	m_bInitialized = LoadShader();
	return m_bInitialized;	
}
void Shader::Uninitialize()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDeleteShader( m_nShaderId );

	m_bInitialized	= false;
	m_nShaderId		= 0;
	m_nProgramId	= 0;
	m_nShaderType	= Vertex;
	m_sShaderFile.clear();
}

bool Shader::LoadShader()
{
	// open the specified shader file
	// read into a string stream
	// acquire an std::string from the stringstream
	// and from that a char**
	// pass this inot the glShaderSource() call for compilation
	// compile the shader	

	using namespace AntiMatter;
	using namespace std;

	if( (m_sShaderFile.length() < 4) || (! Shell::FileExists(m_sShaderFile)) )
	{
		AppLog::Ref().LogMsg( "Shader::LoadShader() - invalid file, or not found %s", m_sShaderFile.c_str() );
		return false;
	}
		
	bool			bRetVal			= false;	
	ifstream		stream;
	stringstream	ssShaderSrc;
	
	try
	{
		stream.open( m_sShaderFile.c_str(), std::ios::in );
		Enforce<AppException> ( (stream.is_open()), string("failed to open stream") );

		ssShaderSrc << stream.rdbuf();		
		stream.close();

		const string & sShaderSrc	= ssShaderSrc.str();
		const char* psShaderSrc		= sShaderSrc.c_str();
		int nSrcLen					= sShaderSrc.size();
				
		// Create the shader
		m_nShaderId = glCreateShader( m_nShaderType );
		Enforce<AppException>( (m_nShaderId != 0), string("shader-id not allocated") );

		// Compile the shader
		glShaderSource( m_nShaderId, 1, &psShaderSrc, (GLint*)&nSrcLen );
		glCompileShader( m_nShaderId );

		// Determine compile status
		GLint nResult = GL_FALSE;
		glGetShaderiv( m_nShaderId, GL_COMPILE_STATUS, &nResult );

		if( nResult == GL_TRUE )
		{
			AppLog::Ref().LogMsg( "Shader::LoadShader(%s) compile successful", m_sShaderFile.c_str() );
		}
		else if( nResult == GL_FALSE )
		{
			GLint		nErrorLength	= 0;
			const int	nMaxLineSize	= 1024;
			char		sLineBuffer[nMaxLineSize];

			glGetShaderInfoLog( m_nShaderId, nMaxLineSize, &nErrorLength, sLineBuffer );

			Enforce<AppException>( (nResult == GL_TRUE), string(sLineBuffer) );
		}

		bRetVal = true;
	}
	catch( AppException & e )
	{
		ATLTRACE(_T("Shader::LoadShader() AppException:\r\n%s"), e.what() );
		AppLog::Ref().LogMsg( "AppException caught in Shader::LoadShader():\r\n%s", e.what() );
	}
	catch( std::bad_alloc & ba )
	{		
		ATLTRACE(_T("Memory allocation exception caught in Shader::LoadShader(), shader not loaded: %s"), ba.what() );		
		AppLog::Ref().LogMsg( "Memory allocation exception caught in Shader::LoadShader(), shader not loaded: %s", ba.what() );
	}
	catch(...)
	{		
		ATLTRACE(_T("Unanticipated exception caught in Shader::LoadShader(), shader not loaded"));
		AppLog::Ref().LogMsg("Unanticipated exception caught in Shader::LoadShader(), shader not loaded");
	}
	
	return bRetVal;
}
