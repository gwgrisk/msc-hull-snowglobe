
#include "stdafx.h"
#include "Snowfall.h"
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

#include <fstream>
#include <iostream>

#include <random>

#include <glex.h>
#include <glext.h>



Snowfall::Snowfall() :
	IGraphNode			( NULL, NULL, std::string("Snowfall") ),
	m_bInitialized		( false ),
	m_nParticleCount	( 0 ),
	m_pParticleData		( NULL ),	
	m_rTimeElapsed		( 0.0f ),
	m_vGravity			( glm::vec3(0.0f) ),
	m_rParticleLifetime	( 0.0f ),
	m_nShaderProgId		( 0 ),
	m_nElFlipYo			( 0 ),
	m_nUpdateSub		( 0 ),
	m_nRenderSub		( 1 ),
	m_rSecsDelta		( 0.0f )
{
}
Snowfall::Snowfall( SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, const std::string & sConfigFile ) :
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	m_sConfigFile		( sConfigFile ),
	m_nParticleCount	( 0 ),
	m_pParticleData 	( NULL ),	
	m_rTimeElapsed		( 0.0f ),
	m_vGravity			( glm::vec3(0.0f) ),
	m_rParticleLifetime	( 0.0f ),
	m_nShaderProgId		( 0 ),
	m_nElFlipYo			( 0 ),
	m_nUpdateSub		( 0 ),
	m_nRenderSub		( 1 ),
	m_rSecsDelta		( 0.0f )
{
	m_bInitialized = Initialize();
}
Snowfall::Snowfall( const Snowfall & rhs ) : 
	IGraphNode			( rhs.m_pGraph, rhs.m_pParent, rhs.m_sId ),
	m_bInitialized		( false ),
	m_sConfigFile		( rhs.m_sConfigFile ),
	m_nParticleCount	( 0 ),
	m_pParticleData		( NULL ),	
	m_rTimeElapsed		( 0.0f ),
	m_vGravity			( glm::vec3(0.0f) ),
	m_rParticleLifetime	( 0.0f ),
	m_nShaderProgId		( 0 ),
	m_nElFlipYo			( 0 ),
	m_nUpdateSub		( 0 ),
	m_nRenderSub		( 1 ),
	m_rSecsDelta		( 0.0f )
{
	if( rhs.Initialized() )
		m_bInitialized = Initialize();
}
Snowfall & Snowfall::operator=( const Snowfall & rhs )
{
	if( this != &rhs )
	{
		if( this->Initialized() )
			Uninitialize();

		m_pGraph		= rhs.m_pGraph;
		m_pParent		= rhs.m_pParent;
		m_sId			= rhs.m_sId;
		m_sConfigFile	= rhs.m_sConfigFile;

		m_nParticleCount	= 0;
		m_pParticleData		= NULL;
		m_rTimeElapsed		= 0.0f;
		m_vGravity			= glm::vec3(0.0f, -9.81f, 0.0f);
		m_rParticleLifetime = 0.0f;
		m_nShaderProgId		= 0;
		m_nElFlipYo			= 0;
		m_nUpdateSub		= 0;
		m_nRenderSub		= 0;
		m_rSecsDelta		= 0;

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
Snowfall::~Snowfall()
{
	Uninitialize();
}

bool Snowfall::Initialize()
{
	if( m_bInitialized )
		return true;

	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using namespace std;

	glex::Load();


	// search the graph for the globe actor, we'll need it for stenciling the snowfall
	m_pGlobe = m_pGraph->GetGraphNodeByName(string("globe"));
	if( ! m_pGlobe )
	{
		AppLog::Ref().LogMsg("%s couldn't find globe object", __FUNCTION__ );
		Uninitialize();
		return false;
	}

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

	if( ! GenerateSnowData2() )
	{
		AppLog::Ref().LogMsg("%s generate snowfall failed", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! CreateShaderProgram() )
	{
		AppLog::Ref().LogMsg("%s create shader program failed", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	m_bInitialized = true;
	return true;
}
void Snowfall::Uninitialize()
{
	m_bInitialized			= false;	
	m_sConfigFile			= "";
	m_rParticleFieldRadius	= 0.0f;
	m_nParticleCount		= 0;
	m_nShaderProgId			= 0;

	m_sAssetFiles.clear();

	// delete particle data
	m_ParticleData2.Uninitialize();
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
	
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0 );
	glUseProgram( 0 );

	if( m_nShaderProgId )
		glDeleteProgram( m_nShaderProgId );

	if( m_nTransformFeedback[0] )
	{
		glDeleteTransformFeedbacks( 2, m_nTransformFeedback );
		ZeroMemory(m_nTransformFeedback, sizeof(GLuint) * 2 );
	}

	if( m_nParticleVBO[0] )
	{
		glDeleteBuffers( 2, m_nParticleVBO );
		ZeroMemory(m_nParticleVBO, sizeof(GLuint) * 2 );
	}

	m_nShaderProgId = 0;

	
}

bool Snowfall::GenerateSnowData2()
{
	if( m_nParticleCount <= 0 )
		return false;

	using AntiMatter::g_Pi;
	using AntiMatter::AppLog;
	using std::variate_generator;
	using std::mt19937;
	using std::uniform_real_distribution;
	using glm::mix;
	

	// Initialize memory buffers for snow particle data
	if( ! m_ParticleData2.Initialized() )	
		if ( ! m_ParticleData2.Initialize( m_nParticleCount ) )
			return false;


	typedef mt19937										Engine;
	typedef uniform_real_distribution<float>			Distribution;
	typedef variate_generator< Engine, Distribution >	Generator;

	Generator r( Engine((DWORD)time(NULL)), Distribution(0.0f, 1.0f) );	

	float rStartTime	= 0.0f;
	float rStagger		= 0.0174f;

	for( unsigned int n = 0; n < m_nParticleCount; n ++ )
	{	
		// create a random start position for the snow particle in the xz plane
		m_ParticleData2.m_ppInitialPosition[n][0] = m_rParticleFieldRadius * cosf(glm::mix(0.0f, 2*g_Pi, r() ));
		m_ParticleData2.m_ppInitialPosition[n][1] = 0.0f;
		m_ParticleData2.m_ppInitialPosition[n][2] = m_rParticleFieldRadius * -sinf(glm::mix(0.0f, 2*g_Pi, r() ));

		// randomly scale the start pos to give an initial velocity for the snow particle in the range 1.1f .. 1.5
		m_ParticleData2.m_ppInitialVelocity[n][0] = glm::mix( 1.15f, 1.45f, r() );
		m_ParticleData2.m_ppInitialVelocity[n][1] = glm::mix( 1.15f, 1.5f, r() );
		m_ParticleData2.m_ppInitialVelocity[n][2] = glm::mix( 1.15f, 1.75f, r() );

		// create a staggered start time for the particle
		m_ParticleData2.m_pParticleStartTime[n] = rStartTime;		
		rStartTime += rStagger;
	}

	// initialize position and velocity data with initial position & velocity
	memcpy( m_ParticleData2.m_ppPosition, m_ParticleData2.m_ppInitialPosition, sizeof(m_ParticleData2.m_ppPosition) );
	memcpy( m_ParticleData2.m_ppVelocity, m_ParticleData2.m_ppInitialVelocity, sizeof(m_ParticleData2.m_ppVelocity) );

	// Set gravity force
	m_vGravity = glm::vec3( 0.0f, -9.81f, 0.0f );

	return true;
}
bool Snowfall::GenerateSnowData()
{
	// generates m_nParticleCount SnowParticles in m_pParticleData

	using AntiMatter::g_Pi;
	using AntiMatter::AppLog;
	using std::variate_generator;
	using std::mt19937;
	using std::uniform_real_distribution;
	using glm::mix;
	
	typedef mt19937										Engine;
	typedef uniform_real_distribution<float>			Distribution;
	typedef variate_generator< Engine, Distribution >	Generator;

	Generator r( Engine((DWORD)time(NULL)), Distribution(0.0f, 1.0f) );

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

		memcpy( pNext->m_rPosition,	pNext->m_rInitialPosition, 3*sizeof(float) );
		memcpy( pNext->m_rVelocity,	pNext->m_rInitialVelocity, 3*sizeof(float) );		

		// create a staggered start time for the particle
		pNext->m_rParticleStartTime = rStartTime;
		rStartTime += rStagger;
	}

	m_vGravity = glm::vec3( 0.0f, -9.81f, 0.0f );


	return true;
}

bool Snowfall::CreateShaderProgram()
{
	// assumes that snow data ( in m_pParticleDataA ) is already created
	using AntiMatter::AppLog;
	
	// Allocate a shader program on the gfx card
	m_nShaderProgId = glCreateProgram();
	if( ! m_nShaderProgId )
	{
		AppLog::Ref().LogMsg( "%s glCreateProgram() failed to acquire a ProgramId", __FUNCTION__ );
		return false;
	}		


	// create shader objects
	Shader v( m_nShaderProgId, g_Cfg.ShadersDir() + ("snowfall.vert"), Vertex );
	Shader f( m_nShaderProgId, g_Cfg.ShadersDir() + ("snowfall.frag"), Fragment );

	if( v.Initialized() && f.Initialized() )
	{
		glAttachShader( m_nShaderProgId, v.GetShaderId() );
		glAttachShader( m_nShaderProgId, f.GetShaderId() );	

		m_vShaders.push_back(v);
		m_vShaders.push_back(f);
	}
	else
	{
		AppLog::Ref().LogMsg( "%s failed to compile shaders", __FUNCTION__ );
		DestroyShaderProgram();
		
		return false;
	}


	// Specify the vertex shader input - e.g. the vertex data (populates m_vShaderArgs)
	CreateShaderArgs(); 
	ApplyShaderArgs();


	// Allocate transform feedbacks and vertex buffers on the gfx card	
	glGenTransformFeedbacks( 2, m_nTransformFeedback );
	glGenBuffers( 2, m_nParticleVBO );

	// Transform feedback buffer 0
	//glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, m_nTransformFeedback[0] );
	//glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_ParticleData2.m_ppPosition
	
	
	int nSize = sizeof( SnowParticle2 ) * m_nParticleCount;
	for ( unsigned int i = 0; i < 2; i ++ )
	{
		// VBOs
		glBindBuffer( GL_ARRAY_BUFFER, m_nParticleVBO[i] );
		glBufferData( GL_ARRAY_BUFFER, nSize, &m_pParticleData[0], GL_DYNAMIC_DRAW );

		// transform feedbacks
		glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, m_nTransformFeedback[i] );
		glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_nParticleVBO[i] );
	}
	/**/


	// Connect vertex shader output vars to transform output buffers
	const char* sVaryings[] = { "Position", "Velocity", "StartTime" };
	int			nNumFields	= sizeof(sVaryings) / sizeof(sVaryings[0]);

	glTransformFeedbackVaryings( m_nShaderProgId, nNumFields, sVaryings, GL_INTERLEAVED_ATTRIBS );

	AppLog::Ref().OutputGlErrors();

	// link shaders	
	GLint nResult = GL_FALSE;
	glLinkProgram( m_nShaderProgId );
	glGetProgramiv( m_nShaderProgId, GL_LINK_STATUS, &nResult );

	if( nResult == GL_TRUE )
	{
		AppLog::Ref().LogMsg( "Shader link stage successful", __FUNCTION__ );

		// acquire subroutine uniforms
		m_nUpdateSub = glGetSubroutineIndex( m_nShaderProgId, GL_VERTEX_SHADER, "update" );
		m_nRenderSub = glGetSubroutineIndex( m_nShaderProgId, GL_VERTEX_SHADER, "render" );
	}
	else if( nResult == GL_FALSE )
	{
		GLint		nErrorLength = 0;
		const int	nMaxLineSize = 1024;
		char		sLineBuffer[nMaxLineSize];
		
		glGetProgramInfoLog( m_nShaderProgId, nMaxLineSize, &nErrorLength, sLineBuffer );		
		AppLog::Ref().LogMsg( "%s failed at linker stage:\r\n%s\n", __FUNCTION__, sLineBuffer );
		
		DestroyShaderProgram();
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 );
	glUseProgram( 0 );	

	return nResult == GL_TRUE ? true:false;
}
void Snowfall::DestroyShaderProgram()
{
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 );
	glUseProgram( 0 );

	if( m_nShaderProgId )
	{
		glDeleteProgram( m_nShaderProgId );
		m_nShaderProgId = 0;
	}

	if( m_nTransformFeedback[0] )
	{
		glDeleteTransformFeedbacks( 2, &m_nTransformFeedback[0] );
		ZeroMemory(&m_nTransformFeedback[0], sizeof(GLuint) * 2 );
	}

	if( m_nParticleVBO[0] )
	{
		glDeleteBuffers( 2, &m_nParticleVBO[0] );
		ZeroMemory(&m_nParticleVBO[0], sizeof(GLuint) * 2 );
	}
}
void Snowfall::CreateShaderArgs()
{
	using std::string;
	
	ShaderArg VertexPosition;
	ShaderArg VertexVelocity;
	ShaderArg ParticleStartTime;
	ShaderArg VertexInitialPosition;
	ShaderArg VertexInitialVelocity;

	m_vShaderArgs.clear();
	m_vShaderArgs.reserve(5);

	VertexPosition.sFieldName			= string("VertexPosition");
	VertexPosition.nFieldSize			= 3;
	VertexPosition.nFieldOffset			= 0;
	VertexPosition.nStride				= sizeof(SnowParticle);

	VertexVelocity.sFieldName			= string("VertexVelocity");
	VertexVelocity.nFieldSize			= 3;
	VertexVelocity.nFieldOffset			= 3;
	VertexVelocity.nStride				= sizeof(SnowParticle);

	ParticleStartTime.sFieldName		= string("ParticleStartTime");
	ParticleStartTime.nFieldSize		= 1;
	ParticleStartTime.nFieldOffset		= 6;
	ParticleStartTime.nStride			= sizeof(SnowParticle);

	VertexInitialPosition.sFieldName	= string("VertexInitialPosition");
	VertexInitialPosition.nFieldSize	= 3;
	VertexInitialPosition.nFieldOffset	= 7;
	VertexInitialPosition.nStride		= sizeof(SnowParticle);

	VertexInitialVelocity.sFieldName	= string("VertexInitialVelocity");
	VertexInitialVelocity.nFieldSize	= 3;
	VertexInitialVelocity.nFieldOffset  = 10;
	VertexInitialVelocity.nStride		= sizeof(SnowParticle);

	m_vShaderArgs.push_back( VertexPosition );
	m_vShaderArgs.push_back( VertexVelocity );
	m_vShaderArgs.push_back( ParticleStartTime );
	m_vShaderArgs.push_back( VertexInitialPosition );
	m_vShaderArgs.push_back( VertexInitialVelocity );
}
void Snowfall::ApplyShaderArgs()
{
	for( GLuint n = 0; n < m_vShaderArgs.size(); n ++)	
		glBindAttribLocation( m_nShaderProgId, n, m_vShaderArgs[n].sFieldName.c_str() );	
	
	for( GLuint n = 0; n < m_vShaderArgs.size(); n ++ )
	{
		glEnableVertexAttribArray(n);
		glVertexAttribPointer( 
			n, 
			m_vShaderArgs[n].nFieldSize,
			GL_FLOAT,
			GL_FALSE,
			m_vShaderArgs[n].nStride,
			(GLfloat *) NULL + m_vShaderArgs[n].nFieldOffset
		);
	}	
}
void Snowfall::SetShaderUniforms()
{
	// Shader Uniforms are:
	// float	SimTime
	// float	FrameDelta
	// vec3		Gravity
	// float	ParticleLifetime
	// mat4		mMVP
	// Texture	Tex

	using AntiMatter::AppLog;
	using std::string;
	using glm::mat4;

	// matrices mModelView, mMVP, mNormal
	mat4 mModelView	= m_pGraph->Cam().V() * m_pParent->GetNodeData().W() * m_Data.W();
	mat4 mMVP		= m_pGraph->Proj().P() * mModelView;
	
	// textures - single texture for now will add more if time permits
	if( ! m_Textures[0] )
		return;

	AssignUniformFloat(		string("SimTime"),			m_rTimeElapsed );
	//AssignUniformFloat(		string("FrameDelta"),		m_rSecsDelta );
	AssignUniformVec3(		string("Gravity"),			m_vGravity );
	AssignUniformFloat(		string("ParticleLifetime"),	m_rParticleLifetime );
	AssignUniformMat4(		string("mMVP"),				mMVP );
	AssignUniformSampler2D( string("Tex"),				m_Textures[0]->TextureId() );
}	 
	
bool Snowfall::AssignUniformFloat( const std::string & sName, const float rVal )
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
bool Snowfall::AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix )
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
bool Snowfall::AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix )
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
bool Snowfall::AssignUniformVec4( const std::string & sName, const glm::vec4 & vector )
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
bool Snowfall::AssignUniformVec3( const std::string & sName, const glm::vec3 & vector )
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
bool Snowfall::AssignUniformSampler2D( const std::string & sName, const GLuint nTexId )
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
HRESULT Snowfall::Update( const float & rSecsDelta )
{
	using glm::mat4;
	using glm::vec3;
	using glm::rotate;
	using glm::translate;

	if( ! m_bInitialized )
		return S_OK;

	if( rSecsDelta < 0.5f )	
		m_rTimeElapsed += rSecsDelta;

	static float rRotation = 0.0f;
	rRotation += 0.15f;

	mat4 mR		= rotate( mat4(1.0), rRotation, vec3(0.0, 1.0, 0.0) );
	mat4 mTr	= translate( mat4(1.0), vec3(0, 200, 0) );
	
	m_Data.W()		= mR * mTr;
	m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();
	
	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() *
						m_Data.Stack();	

	// Update child nodes
	if( IGraphNode::HasChildNodes() )
		IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Snowfall::PreRender()
{
	using AntiMatter::AppLog;

	if( ! m_pGlobe )
		return E_POINTER;

	
	// create a stencil area the shape of the globe	
	Globe* pGlobe = reinterpret_cast<Globe*>(m_pGlobe);
	if( pGlobe->Initialized() )
	{
		glUseProgram( pGlobe->ShaderProgId() );

		glEnable(GL_STENCIL_TEST);
		glColorMask(0, 0, 0, 0);
		glDisable(GL_DEPTH_TEST);
		glStencilFunc( GL_ALWAYS, 0x1, 0x1 );
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

		m_pGlobe->DrawItem();
	
		glColorMask(1, 1, 1, 1);
		glStencilFunc( GL_EQUAL, 1, 1 );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	}

	AppLog::Ref().OutputGlErrors();
	
	
	// -- the Update pass
	glUseProgram(m_nShaderProgId);
	glUniformSubroutinesuiv( GL_VERTEX_SHADER, 1, &m_nUpdateSub );
	
	ApplyShaderArgs();
	SetShaderUniforms();

	glEnable( GL_RASTERIZER_DISCARD );

		glBindBuffer( GL_ARRAY_BUFFER, m_nParticleVBO[m_nElFlipYo] );		
		glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, m_nTransformFeedback[m_nElFlipYo] );

		glBeginTransformFeedback( GL_POINTS );			
			
			glBindVertexArray( m_nParticleVBO[1-m_nElFlipYo] );
			AppLog::Ref().OutputGlErrors();

			glDrawArrays( GL_POINTS, 0, m_nParticleCount );
			AppLog::Ref().OutputGlErrors();

		glEndTransformFeedback();

		ElFlipYo();
		AppLog::Ref().OutputGlErrors();

	glDisable( GL_RASTERIZER_DISCARD );
	
	
	// Set state required for drawing the snow
	glPointSize( 12.0f );
	glEnable(GL_POINT_SPRITE);
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	return S_OK;
}
HRESULT Snowfall::Render()
{
	using AntiMatter::AppLog;

	// -- the Render pass
	glUniformSubroutinesuiv( GL_VERTEX_SHADER, 1, &m_nRenderSub );

	glBindVertexArray( m_nParticleVBO[m_nElFlipYo] );
	glDrawArrays(GL_POINTS, 0, m_nParticleCount );
	glBindVertexArray(0);
	
	AppLog::Ref().OutputGlErrors();

	return S_OK;
}
HRESULT Snowfall::PostRender()
{
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_STENCIL_TEST);

	glBindTexture( GL_TEXTURE_2D,	0 );
	glBindBuffer( GL_ARRAY_BUFFER,	0 );
	glUseProgram(0);

	return S_OK;
}
HRESULT Snowfall::DrawItem()
{
	PreRender();
	Render();
	PostRender();

	return S_OK;
}

// stream support
void Snowfall::AssignAssets( const std::vector<std::string> & sAssets )
{
	// copy assets strings
	m_sAssetFiles.clear();
	m_sAssetFiles.resize( sAssets.size() );
	std::copy( sAssets.begin(), sAssets.end(), m_sAssetFiles.begin() );	
}
bool Snowfall::LoadAssets()
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
std::ostream & operator << ( std::ostream & out, const Snowfall & r )
{
	using AntiMatter::AppLog;
	using std::string;
	using std::vector;

	try
	{
		out << "snowparticles" << " ";
		out << r.ParticleCount() << " ";
		out << r.ParticleFieldRadius() << " ";
		out << r.ParticleLifetime() << " ";

		for( vector<string>::const_iterator n = r.Assets().begin(); n != r.Assets().end(); n ++ )
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
std::istream & operator >> ( std::istream & in, Snowfall & r )
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