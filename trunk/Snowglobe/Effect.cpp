
#include "stdafx.h"

#include <GXBase.h>
#include <AntiMatter\AppLog.h>

#include "Effect.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "Shader.h"

#include <fstream>
#include "slowglobe-cfg.h"

Effect::Effect() : 
	m_bInitialized		( false ),
	m_bAutoBuild		( true ),
	m_sEffectName		( "" ),
	m_nProgramId		( 0 ),
	m_BuildState		( Uncompiled )
{
	glex::Load();
}
Effect::Effect( const std::vector<ShaderDesc> &				Descs, 
				const std::vector<ShaderInputAttribute> &	Attrs, 
				const std::string &							sName, 
				const bool									bAutoBuild ) :
	m_bInitialized		( false ),
	m_bAutoBuild		( bAutoBuild ),
	m_sEffectName		( sName ),
	m_nProgramId		( 0 ),
	m_BuildState		( Uncompiled )
{
	glex::Load();
	m_bInitialized = Initialize( Descs, Attrs );
}
Effect::Effect( const Effect & r ) :
	m_bInitialized		( false ),
	m_bAutoBuild		( r.m_bAutoBuild ),
	m_sEffectName		( r.m_sEffectName ),
	m_nProgramId		( 0 ),
	m_BuildState		( Uncompiled )
{
	using std::vector;
	using std::string;

	// copy ctor
	glex::Load();	

	m_Descs.reserve( r.m_Descs.size() );
	m_Attrs.reserve( r.m_Attrs.size() );

	std::copy( r.m_Attrs.begin(), r.m_Attrs.end(), m_Attrs.begin() );
	std::copy( r.m_Descs.begin(), r.m_Descs.end(), m_Descs.begin() );

	m_bInitialized = Initialize( m_Descs, m_Attrs );
}
Effect & Effect::operator=( const Effect & r )
{
	if( this != &r )
	{
		m_bInitialized	= false;
		m_bAutoBuild	= r.m_bAutoBuild;
		m_sEffectName	= r.m_sEffectName;
		m_nProgramId	= 0;
		m_BuildState	= Uncompiled;

		m_Descs.reserve( r.m_Descs.size() );
		m_Attrs.reserve( r.m_Attrs.size() );

		std::copy( r.m_Attrs.begin(), r.m_Attrs.end(), m_Attrs.begin() );
		std::copy( r.m_Descs.begin(), r.m_Descs.end(), m_Descs.begin() );

		m_bInitialized = Initialize( m_Descs, m_Attrs );
	}

	return *this;
}
Effect::~Effect()
{
	Uninitialize();
}

bool Effect::Initialize(	const std::vector<ShaderDesc> &				Descs, 
							const std::vector<ShaderInputAttribute> &	Attrs )
{
	using AntiMatter::AppLog;

	if( m_bInitialized )
		return false;

	if( Descs.size() <= 0 )
		return false;

	// Create ShaderProgram id
	m_nProgramId = glCreateProgram();
	if( ! m_nProgramId )
	{
		AppLog::Ref().LogMsg( "%s: glCreateProgram() failed to acquire a ProgramId", __FUNCTION__ );
		return false;
	}
	
	// Copy args to member vars for future reference
	m_Descs.resize( Descs.size() );
	std::copy( Descs.begin(), Descs.end(), m_Descs.begin() );

	m_Attrs.resize( Attrs.size() );
	std::copy( Attrs.begin(), Attrs.end(), m_Attrs.begin() );

	// build
	bool bSuccess = true;

	if( m_bAutoBuild )	
		bSuccess = Build();	

	m_bInitialized = bSuccess;

	return bSuccess;
}
void Effect::Uninitialize()
{
	if( ! m_bInitialized )
		return;

	m_bInitialized = false;

	glUseProgram(0);	// this could have performance repercussions. is it wise?

	if( m_nProgramId )
		glDeleteProgram( m_nProgramId );

	m_Attrs.clear();
	m_Descs.clear();
	m_Shaders.clear();
}

bool Effect::Build()
{
	bool bSuccess = false;

	if( m_BuildState == Linked )
		return false;

	if( CompileShaders() )
	{
		SetInputAttributes();
		bSuccess = Link();
	}

	return bSuccess;
}
bool Effect::CompileShaders()
{
	using AntiMatter::AppLog;

	// Create Shaders
	for( std::vector<ShaderDesc>::iterator x = m_Descs.begin(); x != m_Descs.end(); x ++ )
	{
		ShaderDesc & next = (*x);
		Shader s = Shader(m_nProgramId, g_Cfg.ShadersDir() + next.sFileName, next.nType);

		if( s.Initialized() )
		{
			glAttachShader( m_nProgramId, s.GetShaderId() );
			m_Shaders.push_back(s);
		}
		else
		{
			AppLog::Ref().LogMsg( "Effect %s: compilation failed", m_sEffectName );
			m_BuildState = Uncompiled;
			m_Shaders.clear();
			return false;
		}		
	}	

	// Update state
	m_BuildState = Compiled;

	return true;
}
bool Effect::SetInputAttributes()
{
	// specify shader attribute locations
	for( GLuint n = 0; n < m_Attrs.size(); n ++)
		glBindAttribLocation( m_nProgramId, n, m_Attrs[n].sFieldName.c_str() );

	return true;
}
bool Effect::Link()
{
	using AntiMatter::AppLog;

	// link the shaders
	GLint nResult = GL_FALSE;
	glLinkProgram( m_nProgramId );
	glGetProgramiv( m_nProgramId, GL_LINK_STATUS, &nResult );

	if( nResult == GL_TRUE )
	{
		AppLog::Ref().LogMsg( "%s: ShaderProgram link successful", __FUNCTION__ );
		m_BuildState = Linked;
	}
	else if( nResult == GL_FALSE )
	{
		GLint		nErrorLength = 0;
		const int	nMaxLineSize = 1024;
		char		sLineBuffer[nMaxLineSize];
		
		glGetProgramInfoLog( m_nProgramId, nMaxLineSize, &nErrorLength, sLineBuffer );
		
		AppLog::Ref().LogMsg( "%s: failed at linker stage:\r\n%s\r\n", __FUNCTION__, sLineBuffer );		
		return false;
	}

	return true;
}
	
bool Effect::AssignUniformInt( const std::string & sName, const int rVal )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;	

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );	

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		return false;
	}
	
	glUniform1i( nLocation, rVal );
	return true;
}
bool Effect::AssignUniformFloat( const std::string & sName, const float rVal )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;	

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );	

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		return false;
	}
	
	glUniform1f( nLocation, rVal );
	return true;
}
	
bool Effect::AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;	

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );
	
	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		return false;
	}

	if( nLocation >= 0 )	
		glUniformMatrix4fv( nLocation, 1, GL_FALSE, &matrix[0][0] );

	return true;
}
bool Effect::AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;	

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );
	
	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		return false;
	}

	glUniformMatrix3fv( nLocation, 1, GL_FALSE, &matrix[0][0] );

	return true;
}
	
bool Effect::AssignUniformVec4( const std::string & sName, const glm::vec4 & vector )
{	
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		// ATLASSERT(0);
		return false;
	}

	glUniform4fv( nLocation, 1, &vector[0] );	
	return true;
}
bool Effect::AssignUniformVec3( const std::string & sName, const glm::vec3 & vector )
{	
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		//ATLASSERT(0);
		return false;
	}

	glUniform3fv( nLocation, 1, &vector[0] );
	return true;
}

bool Effect::AssignUniformVec2( const std::string & sName, const glm::vec2 & vector )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		//ATLASSERT(0);
		return false;
	}

	glUniform2fv( nLocation, 1, &vector[0] );
	return true;
}
	
bool Effect::AssignUniformSampler2D( const std::string & sName, const GLuint nTexId )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;

	GLuint nLocation = glGetUniformLocation( this->m_nProgramId, sName.c_str() );

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		return false;
	}

	glActiveTexture( GL_TEXTURE0 + nTexId );
	glBindTexture( GL_TEXTURE_2D, nTexId );
	
	glUniform1i( nLocation, nTexId );

	AppLog::Ref().OutputGlErrors();
	return true;
}
	
//std::ofstream & operator << ( std::ofstream & out, const Effect & r )
//{
//	return out;
//}
//std::ifstream & operator >> ( std::ifstream & in, Effect & r )
//{
//	return in;
//}