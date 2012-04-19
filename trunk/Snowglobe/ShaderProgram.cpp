
#include "stdafx.h"

#include "ShaderProgram.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "Shader.h"
#include "VertexClass.h"

#include <vector>
#include <string>
#include <AntiMatter\AppLog.h>

#include <fstream>

ShaderProgram::ShaderProgram(	CustomVertex** ppVertices, 
								DWORD* pdwIndices, 
								GLuint nVertices, 
								GLuint nIndices, 
								const std::vector<ShaderDesc> & shaders, 
								const std::vector<ShaderInputAttribute> & args )  :
	m_nProgramId		( 0 ),
	m_bInitialized		( false ),	
	m_nVaoId			( 0 ),
	m_nVboId			( 0 ),
	m_nVboIndexId		( 0 ),
	m_ppVertices		( ppVertices ),
	m_pdwIndices		( pdwIndices ),
	m_nVertexCount		( nVertices ),
	m_nIndexCount		( nIndices )
{
	m_bInitialized = Initialize( shaders, args );

	if( ! m_bInitialized ) 
		Uninitialize();
}
ShaderProgram::~ShaderProgram()
{
	try{ Uninitialize(); }catch(...){ /* Parasoft pacification */ }
}

bool ShaderProgram::Initialize( const std::vector<ShaderDesc> & vShaderDescs, const std::vector<ShaderInputAttribute> & vShaderArgs )
{
	using AntiMatter::AppLog;
	using std::vector;
	using std::string;

	typedef vector<ShaderDesc>::const_iterator				Itr;
	typedef vector<Shader>::iterator						SItr;
	typedef vector<ShaderInputAttribute>::const_iterator	AItr;

	bool bRetVal = true;

	// Acquire a ProgramId for this ShaderProgram
	m_nProgramId = glCreateProgram();
	if( ! m_nProgramId )
	{		
		AppLog::Ref().LogMsg( "ShaderProgram::Initialize(): glCreateProgram() failed to acquire a ProgramId" );		
		return false;
	}

	// Create a (static) vertex buffer and index buffer on the gfx card for the vertex/index data
	CopyToGfxMem();
		
	// Create Shaders
	for( Itr x = vShaderDescs.begin(); x != vShaderDescs.end(); x ++ )
	{
		Shader s( m_nProgramId, x->sFileName, x->nType );

		if( s.Initialized() )
		{
			glAttachShader( m_nProgramId, s.GetShaderId() );
			m_Shaders.push_back(s);
		}
		else
		{
			AppLog::Ref().LogMsg("ShaderProgram::Initialize() - All shader compilation failed");
			return false;
		}
	}


	// Create and bind to a vertex array object, which stores the relationship between 
	// the buffer and the input attributes	
	glGenVertexArrays( 1, &m_nVaoId );
	glBindVertexArray( m_nVaoId );

	AppLog::Ref().OutputGlErrors();
	
	/*
	// Specify the shader arg locations (e.g. their order in the shader code)
	for( GLuint n = 0; n < vShaderArgs.size(); n ++)
		glBindAttribLocation( m_nProgramId, n, vShaderArgs[n].sFieldName.c_str() );
	
	AppLog::Ref().OutputGlErrors();
	*/
		
	glBindBuffer( GL_ARRAY_BUFFER,			m_nVboId );		
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	m_nVboIndexId );	
	
	// vertex attributes ( describe attributes )
	for( GLuint n = 0; n < vShaderArgs.size(); n ++ )
	{
		// Enable the vertex attribute array (we're using interleaved array, since its faster)	
		glEnableVertexAttribArray(n);	
		glVertexAttribPointer( 
			n, 
			vShaderArgs[n].nFieldSize,
			GL_FLOAT,
			GL_FALSE,
			vShaderArgs[n].nStride,
			(GLfloat *) NULL + vShaderArgs[n].nFieldOffset
		);

		AppLog::Ref().OutputGlErrors();
	}

	glBindVertexArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	
	// link the shaders	
	GLint nResult = GL_FALSE;
	glLinkProgram( m_nProgramId );
	glGetProgramiv( m_nProgramId, GL_LINK_STATUS, &nResult );

	if( nResult == GL_TRUE )
	{
		AppLog::Ref().LogMsg( "ShaderProgram::Initialize(): ShaderProgram link successful" );
	}
	else if( nResult == GL_FALSE )
	{
		GLint		nErrorLength = 0;
		const int	nMaxLineSize = 1024;
		char		sLineBuffer[nMaxLineSize];
		
		glGetProgramInfoLog( m_nProgramId, nMaxLineSize, &nErrorLength, sLineBuffer );
		
		AppLog::Ref().LogMsg( "ShaderProgram::Initialize() failed at linker stage:\r\n%s", sLineBuffer );		
		bRetVal = false;
	}

	
	AppLog::Ref().OutputGlErrors();

	return bRetVal;
}
void ShaderProgram::Uninitialize()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);

	if( m_nProgramId )
		glDeleteProgram( m_nProgramId );

	if( m_nVaoId )
		glDeleteVertexArrays( 1, &m_nVaoId );

	if( m_nVboId )
		glDeleteBuffers( 1, &m_nVboId );

	if( m_nVboIndexId )
		glDeleteBuffers( 1, &m_nVboIndexId );

	m_Shaders.clear();	

	m_nProgramId	= 0;
	m_nVaoId		= 0;
	m_nVboId		= 0;
	m_nVboIndexId	= 0;	
	m_bInitialized	= false;
}
void ShaderProgram::CopyToGfxMem()
{
	glGenBuffers( 1, &m_nVboId );							// request a VBO id allocation
	glBindBuffer( GL_ARRAY_BUFFER, m_nVboId );				// bind the ident with the mem location
	
	glBufferData(											// allocate the space in gfx memory
		GL_ARRAY_BUFFER,
		sizeof(CustomVertex) * m_nVertexCount,
		0,
		GL_STATIC_DRAW
	);
	
	glBufferSubData(										// copy vertex data to gfx mem
		GL_ARRAY_BUFFER,
		0,
		sizeof(CustomVertex) * m_nVertexCount,
		*m_ppVertices
	);

	if( m_nIndexCount > 0 )
	{
		// VBO for indices
		glGenBuffers( 1, &m_nVboIndexId );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_nVboIndexId );

		// allocate & memcpy
		glBufferData( 
			GL_ELEMENT_ARRAY_BUFFER,
			sizeof(DWORD) * m_nIndexCount,
			m_pdwIndices, 
			GL_STATIC_DRAW
		);
	}
	
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

bool ShaderProgram::AssignUniformFloat( const std::string & sName, const float rVal )
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
	
	glUniform1f( nLocation, rVal );
	return true;
}
bool ShaderProgram::AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix )
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

	if( nLocation >= 0 )	
		glUniformMatrix4fv( nLocation, 1, GL_FALSE, &matrix[0][0] );

	return true;
}
bool ShaderProgram::AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix )
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

	glUniformMatrix3fv( nLocation, 1, GL_FALSE, &matrix[0][0] );

	return true;
}
bool ShaderProgram::AssignUniformVec4( const std::string & sName, const glm::vec4 & vector )
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
bool ShaderProgram::AssignUniformVec3( const std::string & sName, const glm::vec3 & vector )
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
bool ShaderProgram::AssignUniformSampler2D( const std::string & sName, const GLuint nTexId )
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

	glActiveTexture( GL_TEXTURE0 + nTexId );
	glBindTexture( GL_TEXTURE_2D, nTexId );
	
	glUniform1i( nLocation, nTexId );

	AppLog::Ref().OutputGlErrors();
	return true;
}