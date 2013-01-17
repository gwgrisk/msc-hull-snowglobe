
#include "stdafx.h"
#include "Snow.h"
#include "Globe.h"
#include "SceneGraph.h"

#include <iomanip>
#include <fstream>
#include "slowglobe-cfg.h"

#include <AntiMatter\constants.h>
#include <AntiMatter\Exceptions.h>
#include <AntiMatter\ShellUtils.h>
#include <AntiMatter\AppLog.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <fstream>
#include <iostream>


#include <random>
#include <functional>


Snow::Snow() :
	IGraphNode			( NULL, NULL, std::string("Snow") ),
	m_bInitialized		( false ),
	m_nParticleCount	( 0 ),
	m_pParticleData		( NULL ),
	m_rTimeElapsed		( 0.0f ),
	m_vGravity			( glm::vec3(0.0f) ),
	m_rParticleLifetime	( 0.0f ),
	m_nShaderProgId		( 0 ),
	m_nVboId			( 0 ),
	m_pGlobe			( NULL )
{
}
Snow::Snow( SceneGraph* pGraph, IGraphNode* pParent, IGraphNode* pGlobe, const std::string & sId, const std::string & sConfigFile ) :
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	m_sConfigFile		( sConfigFile ),
	m_nParticleCount	( 0 ),
	m_pParticleData		( NULL ),
	m_rTimeElapsed		( 0.0f ),
	m_vGravity			( glm::vec3(0.0f) ),
	m_rParticleLifetime	( 0.0f ),
	m_nShaderProgId		( 0 ),
	m_nVboId			( 0 ),
	m_pGlobe			( pGlobe )
{
	m_bInitialized = Initialize();
}
Snow::Snow( const Snow & rhs ) : 
	IGraphNode			( rhs.m_pGraph, rhs.m_pParent, rhs.m_sId ),
	m_bInitialized		( false ),
	m_sConfigFile		( rhs.m_sConfigFile ),
	m_nParticleCount	( 0 ),
	m_pParticleData		( NULL ),
	m_rTimeElapsed		( 0.0f ),
	m_vGravity			( glm::vec3(0.0f) ),
	m_rParticleLifetime	( 0.0f ),
	m_nShaderProgId		( 0 ),
	m_nVboId			( 0 ),
	m_pGlobe			( rhs.m_pGlobe )
{
	if( rhs.Initialized() )
		m_bInitialized = Initialize();
}
Snow & Snow::operator=( const Snow & rhs )
{
	if( this != &rhs )
	{
		if( this->Initialized() )
			Uninitialize();

		m_pGraph		= rhs.m_pGraph;
		m_pParent		= rhs.m_pParent;
		m_sId			= rhs.m_sId;
		m_sConfigFile	= rhs.m_sConfigFile;
		m_pGlobe		= rhs.m_pGlobe;

		if( rhs.Initialized() )
			Initialize();
		else
		{
			m_bInitialized = false;
			Uninitialize();
		}
	}

	return *this;
}
Snow::~Snow()
{
	Uninitialize();
}

bool Snow::Initialize()
{
	if( m_bInitialized )
		return true;

	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using namespace std;
	glex::Load();

	if( ! m_pGlobe )
		return false;

	// attempt to load data from config file
	if( FileExists( m_sConfigFile ) )
	{
		ifstream in;
		in.open( m_sConfigFile, std::ios::in );

		if( in.good() )
		{
			in >> *this;
			in.close();
		}
		else
		{
			AppLog::Ref().LogMsg("%s config file invalid", __FUNCTION__ );
			Uninitialize();
			return false;
		}
	}
	else
	{
		AppLog::Ref().LogMsg("%s config file not found", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! LoadAssets() )
	{
		AppLog::Ref().LogMsg("%s load assets failed", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! GenerateSnowData() )
	{
		AppLog::Ref().LogMsg("%s generate snowfall failed", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! CreateShader() )
	{
		AppLog::Ref().LogMsg("%s create shader program failed", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	/*
	m_pGlobe = m_pGraph->GetGraphNodeByName(string("globe"));
	if( ! m_pGlobe )
	{
		AppLog::Ref().LogMsg("%s couldn't find globe object", __FUNCTION__ );
		Uninitialize();
		return false;
	}
	*/

	m_bInitialized = true;
	return true;
}
void Snow::Uninitialize()
{
	m_bInitialized			= false;	
	m_sConfigFile			= "";
	m_rParticleFieldRadius	= 0.0f;
	m_nParticleCount		= 0;
	m_nShaderProgId			= 0;
	m_nVboId				= 0;

	m_sAssetFiles.clear();

	// delete particle data
	if( m_pParticleData )
	{
		delete [] m_pParticleData;
		m_pParticleData = NULL;
	}

	// delete textures
	for( unsigned int n = 0; n < m_Textures.size(); n ++ )
	{
		if( m_Textures[n] )
			delete m_Textures[n];
	}

	m_vShaderArgs.clear();
	m_vShaders.clear();
	
	if( m_nShaderProgId )
		glDeleteProgram( m_nShaderProgId );

	if( m_nVboId )
		glDeleteBuffers( 1, &m_nVboId );
}

bool Snow::GenerateSnowData()
{
	using AntiMatter::g_Pi;
	using AntiMatter::AppLog;	
	using std::mt19937;
	using std::uniform_real_distribution;
	using glm::mix;
	
	typedef mt19937										Engine;
	typedef uniform_real_distribution<float>			Distribution;	

	// Generator r( Engine((DWORD)time(NULL)), Distribution(0.0f, 1.0f) );
	auto r = std::bind(Distribution(0.0f, 1.0f), Engine((DWORD)time(NULL)));

	if( m_pParticleData )
	{
		delete [] m_pParticleData;
		m_pParticleData = NULL;
	}

	m_pParticleData = new (std::nothrow) SnowParticle[m_nParticleCount];
	if( ! m_pParticleData )
		return false;

	float rStartTime	= 0.0f;
	float rStagger		= 0.0174f;

	for( unsigned int n = 0; n < m_nParticleCount; n ++ )
	{
		SnowParticle* pNext = &m_pParticleData[n];
		
		// create a random start position for the snow particle in the xz plane
		pNext->m_rInitialPosition[0] = m_rParticleFieldRadius * cosf(glm::mix(0.0f, 2*g_Pi, r() ));
		pNext->m_rInitialPosition[1] = 0.0f;
		pNext->m_rInitialPosition[2] = m_rParticleFieldRadius * -sinf(glm::mix(0.0f, 2*g_Pi, r() ));
		

		// randomly scale the start pos to give an initial velocity for the snow particle in the range 1.1f .. 1.5
		pNext->m_rInitialVelocity[0] = glm::mix( 1.15f, 1.45f, r() );
		pNext->m_rInitialVelocity[1] = glm::mix( 1.15f, 1.5f, r() );
		pNext->m_rInitialVelocity[2] = glm::mix( 1.15f, 1.75f, r() );
		

		// create a staggered start time for the particle
		pNext->m_rStartTime = rStartTime;
		rStartTime += rStagger;
	}

	m_vGravity = glm::vec3( 0.0f, -9.81, 0.0f );


	return true;
}

bool Snow::CreateShader()
{
	using AntiMatter::AppLog;
	
	int nSize = sizeof(SnowParticle) * m_nParticleCount;

	// create program
	m_nShaderProgId = glCreateProgram();
	if( ! m_nShaderProgId )
	{		
		AppLog::Ref().LogMsg( "%s glCreateProgram() failed to acquire a ProgramId", __FUNCTION__ );
		return false;
	}

	// copy data to gfx mem
	glGenBuffers( 1, &m_nVboId );									// request a VBO id allocation
	glBindBuffer( GL_ARRAY_BUFFER, m_nVboId );						// bind the ident with the mem location
		
	glBufferData( GL_ARRAY_BUFFER, nSize, 0, GL_STATIC_DRAW );		// allocate gfx mem	
	glBufferSubData( GL_ARRAY_BUFFER, 0, nSize, m_pParticleData );	// copy to gfx mem

	AppLog::Ref().OutputGlErrors();

	
	// create shader objects
	Shader v( m_nShaderProgId, g_Cfg.ShadersDir() + ("Snow.vert"), Vertex );
	Shader f( m_nShaderProgId, g_Cfg.ShadersDir() + ("Snow.frag"), Fragment );

	if( v.Initialized() && f.Initialized() )
	{
		glAttachShader( m_nShaderProgId, v.GetShaderId() );
		glAttachShader( m_nShaderProgId, f.GetShaderId() );	
	}
	else
	{
		AppLog::Ref().LogMsg( "%s failed to compile shaders", __FUNCTION__ );
		
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glUseProgram( 0 );

		if( m_nShaderProgId )
			glDeleteProgram( m_nShaderProgId );

		if( m_nVboId )
			glDeleteBuffers( 1, &m_nVboId );

		return false;
	}

	m_vShaders.push_back(v);
	m_vShaders.push_back(f);


	// Specify the shader arg locations (e.g. their order in the shader code)
	CreateShaderArgs();
	for( GLuint n = 0; n < m_vShaderArgs.size(); n ++)
		glBindAttribLocation( m_nShaderProgId, n, m_vShaderArgs[n].sFieldName.c_str() );	

	AppLog::Ref().OutputGlErrors();

	glBindBuffer( GL_ARRAY_BUFFER, m_nVboId );

	// specify particle data format
	for( GLuint n = 0; n < m_vShaderArgs.size(); n ++ )
	{
		// Enable the vertex attribute array (we're using interleaved array, since its faster)
		glEnableVertexAttribArray(n);	
		glVertexAttribPointer( 
			n, 
			m_vShaderArgs[n].nFieldSize,
			GL_FLOAT,
			GL_FALSE,
			m_vShaderArgs[n].nStride,
			(GLfloat *) NULL + m_vShaderArgs[n].nFieldOffset
		);

		AppLog::Ref().OutputGlErrors();
	}

	// link shaders	
	GLint nResult = GL_FALSE;
	glLinkProgram( m_nShaderProgId );
	glGetProgramiv( m_nShaderProgId, GL_LINK_STATUS, &nResult );

	if( nResult == GL_TRUE )
	{
		AppLog::Ref().LogMsg( "Shader link stage successful", __FUNCTION__ );
	}
	else if( nResult == GL_FALSE )
	{
		GLint		nErrorLength = 0;
		const int	nMaxLineSize = 1024;
		char		sLineBuffer[nMaxLineSize];
		
		glGetProgramInfoLog( m_nShaderProgId, nMaxLineSize, &nErrorLength, sLineBuffer );
		
		AppLog::Ref().LogMsg( "%s failed at linker stage:\r\n%s", __FUNCTION__, sLineBuffer );
		
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glUseProgram( 0 );

		if( m_nShaderProgId )
			glDeleteProgram( m_nShaderProgId );

		if( m_nVboId )
			glDeleteBuffers( 1, &m_nVboId );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	AppLog::Ref().OutputGlErrors();

	return nResult == GL_TRUE ? true:false;
}
void Snow::CreateShaderArgs()
{
	using std::string;

	m_vShaderArgs.clear();
	m_vShaderArgs.reserve(5);

	ShaderArg InitialVelocity;
	ShaderArg InitialPosition;
	ShaderArg ParticleStartTime;

	InitialPosition.sFieldName		= string("InitialPosition");
	InitialPosition.nFieldSize		= 3;	// not bytes, num of components
	InitialPosition.nFieldOffset	= 0;
	InitialPosition.nStride			= sizeof(SnowParticle);

	InitialVelocity.sFieldName		= string("InitialVelocity");
	InitialVelocity.nFieldSize		= 3;	// not bytes, num of components
	InitialVelocity.nFieldOffset	= 3;
	InitialVelocity.nStride			= sizeof(SnowParticle);

	ParticleStartTime.sFieldName	= string("ParticleStartTime");
	ParticleStartTime.nFieldSize	= 1;	// not bytes, num of components
	ParticleStartTime.nFieldOffset	= 6;
	ParticleStartTime.nStride		= sizeof(SnowParticle);	

	m_vShaderArgs.push_back( InitialPosition );
	m_vShaderArgs.push_back( InitialVelocity );
	m_vShaderArgs.push_back( ParticleStartTime );
}
void Snow::SetShaderUniforms()
{
	using AntiMatter::AppLog;
	using std::string;
	using glm::mat4;

	// matrices mModelView, mMVP, mNormal
	mat4 mModelView	= m_pGraph->Cam().V() * m_pParent->GetNodeData().W() * m_Data.W();
	mat4 mMVP		= m_pGraph->Proj().P() * mModelView;

	glUseProgram( m_nShaderProgId );
	glBindBuffer( GL_ARRAY_BUFFER, m_nVboId );		// Particles
	
	// textures - single texture for now will add more if time permits
	Texture* pTexture = m_Textures[0];
	if( ! pTexture )
		return;

	GLuint nTexId = pTexture->TextureId();

	AssignUniformSampler2D( string("snowflake"),		nTexId );
	AssignUniformFloat(		string("TimeElapsed"),		m_rTimeElapsed );
	AssignUniformVec3(		string("Gravity"),			m_vGravity );
	AssignUniformFloat(		string("ParticleLifetime"),	m_rParticleLifetime );
	AssignUniformMat4(		string("mMVP"),				mMVP );

	AppLog::Ref().OutputGlErrors();
}	 
	
bool Snow::AssignUniformFloat( const std::string & sName, const float rVal )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;	

	GLuint nLocation = glGetUniformLocation( m_nShaderProgId, sName.c_str() );	

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		ATLASSERT(0);
		return false;
	}
	
	glUniform1f( nLocation, rVal );
	return true;
}
bool Snow::AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;	

	GLuint nLocation = glGetUniformLocation( m_nShaderProgId, sName.c_str() );
	
	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		ATLASSERT(0);
		return false;
	}

	if( nLocation >= 0 )	
		glUniformMatrix4fv( nLocation, 1, GL_FALSE, &matrix[0][0] );

	return true;
}
bool Snow::AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;	

	GLuint nLocation = glGetUniformLocation( m_nShaderProgId, sName.c_str() );
	
	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		ATLASSERT(0);
		return false;
	}

	glUniformMatrix3fv( nLocation, 1, GL_FALSE, &matrix[0][0] );

	return true;
}
bool Snow::AssignUniformVec4( const std::string & sName, const glm::vec4 & vector )
{	
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;

	GLuint nLocation = glGetUniformLocation( m_nShaderProgId, sName.c_str() );

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		ATLASSERT(0);
		return false;
	}

	glUniform4fv( nLocation, 1, &vector[0] );	
	return true;
}
bool Snow::AssignUniformVec3( const std::string & sName, const glm::vec3 & vector )
{	
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;

	GLuint nLocation = glGetUniformLocation( m_nShaderProgId, sName.c_str() );

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		ATLASSERT(0);
		return false;
	}

	glUniform3fv( nLocation, 1, &vector[0] );
	return true;
}
bool Snow::AssignUniformSampler2D( const std::string & sName, const GLuint nTexId )
{
	using namespace AntiMatter;

	if( ! m_bInitialized )
		return false;

	GLuint nLocation = glGetUniformLocation( m_nShaderProgId, sName.c_str() );

	if( nLocation == 0xffffffff )
	{
		AppLog::Ref().OutputGlErrors();
		ATLASSERT(0);
		return false;
	}

	glActiveTexture( GL_TEXTURE0 + nTexId );
	glBindTexture( GL_TEXTURE_2D, nTexId );
	
	glUniform1i( nLocation, nTexId );

	AppLog::Ref().OutputGlErrors();
	return true;
}

// IGraphNode
HRESULT Snow::Update( const float & rSecsDelta )
{
	using glm::mat4;
	using glm::vec3;
	using glm::rotate;
	using glm::translate;

	if( ! m_bInitialized )
		return S_OK;

	if( rSecsDelta < 0.5f )	
		m_rTimeElapsed += rSecsDelta;

	if( m_rTimeElapsed > m_rParticleLifetime )
		m_rTimeElapsed = 0.0f;

	static float rRotation = 0.0f;
	rRotation += 0.15f;

	mat4 mR		= rotate( mat4(1.0), rRotation, vec3(0.0, 1.0, 0.0) );
	mat4 mTr	= translate( mat4(1.0), vec3(0, 300, 0) );
	
	m_Data.W()		= mR * mTr;
	m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();
	
	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() *
						m_Data.Stack();	

	// Update child nodes
	IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Snow::PreRender()
{
	using AntiMatter::AppLog;

	if( ! m_pGlobe )
		return E_POINTER;

	// create a stencil area the shape of the globe	
	glEnable(GL_STENCIL_TEST);
	glColorMask(0, 0, 0, 0);
	glDisable(GL_DEPTH_TEST);
	glStencilFunc( GL_ALWAYS, 0x1, 0x1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	m_pGlobe->DrawItem();
	
	glColorMask(1, 1, 1, 1);
	glStencilFunc( GL_EQUAL, 1, 1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

	// Set state required for drawing the snow
	SetShaderUniforms();
	glPointSize( 10.0f );
	glEnable( GL_POINT_SPRITE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	return S_OK;
}
HRESULT Snow::Render()
{		
	glDrawArrays( GL_POINTS, 0, m_nParticleCount );	
	return S_OK;
}
HRESULT Snow::PostRender()
{
	using AntiMatter::AppLog;

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_STENCIL_TEST);
	

	AppLog::Ref().OutputGlErrors();	

	// Release buffers set in SetShaderArgs()
	glBindTexture( GL_TEXTURE_2D,	0 );
	glBindBuffer( GL_ARRAY_BUFFER,	0 );
	glUseProgram(0);

	return S_OK;
}
HRESULT Snow::DrawItem()
{
	PreRender();
	Render();
	PostRender();

	return S_OK;
}

// stream support
void Snow::AssignAssets( const std::vector<std::string> & sAssets )
{
	// copy assets strings
	m_sAssetFiles.clear();
	m_sAssetFiles.resize( sAssets.size() );
	std::copy( sAssets.begin(), sAssets.end(), m_sAssetFiles.begin() );	
}
bool Snow::LoadAssets()
{
	// LoadAssets() assumes that the asset vector has already been initialized 
	// via the streaming operator (this is done via RAII in Initialize()).
	using AntiMatter::Shell::FileExists;
	using AntiMatter::AppLog;
	using std::string;
	using std::vector;
	typedef vector<string> Assets;

	for( unsigned int n = 0; n < m_sAssetFiles.size(); n ++ )
	{
		string sTexture = g_Cfg.AssetsDir()  + m_sAssetFiles[n];

		if( FileExists(sTexture) )
		{
			Texture* pTexture = new (std::nothrow) Texture(sTexture);
			if( ! pTexture || ! pTexture->Initialized() )
			{
				AppLog::Ref().LogMsg( "%s failed to load texture %s", __FUNCTION__, sTexture.c_str() );
				
				if( pTexture )
					delete pTexture;

				return false;
			}

			m_Textures.push_back( pTexture );
		}
	}

	return true;
}
	
// streaming
std::ostream & operator << ( std::ostream & out, const Snow & r )
{
	using AntiMatter::AppLog;
	using std::string;

	try
	{
		out << "snowparticles" << " ";
		out << r.ParticleCount() << " ";
		out << r.ParticleFieldRadius() << " ";
		out << r.ParticleLifetime() << " ";

		for( std::vector<std::string>::const_iterator n = r.Assets().begin(); n != r.Assets().end(); n ++ )
		{
			if( n != r.Assets().begin() )
				 out << " ";

			string sNext = (*n);
			out << sNext;
		}
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated exception encountered (invalid config file or disk issue)", __FUNCTION__ );		
	}
	
	return out;
}
std::istream & operator >> ( std::istream & in, Snow & r )
{
	using AntiMatter::AppLog;
	using std::vector;
	using std::string;
	

	try
	{
		char			sBuffer[256];
		vector<string>	sAssets;
		int				n			= 0;
		int				nNumParticles;
		float			rParticleFieldRadius;
		float			rParticleLifetime;

		in >> sBuffer;
		
		if( strcmp(sBuffer, "snowparticles") == 0 )
		{
			sAssets.reserve(20);

			in >> nNumParticles;
			in >> rParticleFieldRadius;
			in >> rParticleLifetime;
			
			while( in.good() )
			{
				in >> sBuffer;
				sAssets.push_back( string(sBuffer) );
				n++;
			}
			
			r.ParticleCount( nNumParticles );
			r.ParticleFieldRadius( rParticleFieldRadius );
			r.ParticleLifetime( rParticleLifetime );
			r.AssignAssets( sAssets );
		}
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated exception encountered (invalid config file or disk issue)", __FUNCTION__ );
	}

	return in;
}