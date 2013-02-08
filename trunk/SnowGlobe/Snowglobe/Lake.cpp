
#include "stdafx.h"

#include "Lake.h"
#include "IGraphNode.h"

#include "Plane.h"

#include "SeasonalTimeline.h"
#include "Vbo.h"
#include "SceneGraph.h"
#include "VertexClass.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"

#include <random>
#include <functional>
#include <iomanip>
#include <fstream>
#include <iostream>

#include "slowglobe-cfg.h"

#include <AntiMatter\constants.h>
#include <AntiMatter\Exceptions.h>
#include <AntiMatter\ShellUtils.h>
#include <AntiMatter\AppLog.h>

#include <glm\glm.hpp>


Lake::Lake() :
	IGraphNode			( NULL, NULL, std::string("lake") ),
	m_bInitialized		( false ),
	m_pEffect			( NULL ),
	m_pVbo				( NULL ),
	m_nVaoId			( 0 ),
	m_rWidth			( 0.0f ),
	m_rHeight			( 0.0f ),
	m_nNumWaves			( 4 )
{
}
Lake::Lake(	SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, 
		const float rWidth, const float rHeight, 
		const std::string & sTexture,
		const std::string & sAlphaMap,
		const std::string & sNormalMap ) :
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	m_pEffect			( NULL ),
	m_pVbo				( NULL ),
	m_nVaoId			( 0 ),
	m_sTextureMap		( sTexture ),
	m_sAlphaMap			( sAlphaMap ),
	m_sNormalMap		( sNormalMap ),
	m_rWidth			( rWidth ),
	m_rHeight			( rHeight ),
	m_nNumWaves			( 3 )
{
	Initialize();
}
Lake::Lake( const Lake & r ) : 
	IGraphNode		( r.m_pGraph, r.m_pParent, r.m_sId ),
	m_bInitialized	( false ),
	m_pEffect		( NULL ),
	m_pVbo			( NULL ),
	m_nVaoId		( 0 ),
	m_sTextureMap	( r.m_sTextureMap ),
	m_sAlphaMap		( r.m_sAlphaMap ),
	m_sNormalMap	( r.m_sNormalMap ),
	m_rWidth		( r.m_rWidth ),
	m_rHeight		( r.m_rHeight ),
	m_nNumWaves		( r.m_nNumWaves )
{	
	if( r.Initialized() )
		Initialize();
}
Lake & Lake::operator=( const Lake & r )
{
	if( this != &r )
	{
		Uninitialize();

		m_pGraph		= r.m_pGraph;
		m_pParent		= r.m_pParent;
		m_sId			= r.m_sId;

		m_sTextureMap	= r.m_sTextureMap;
		m_sAlphaMap		= r.m_sAlphaMap;
		m_sNormalMap	= r.m_sNormalMap;
		m_rWidth		= r.m_rWidth;
		m_rHeight		= r.m_rHeight;
		m_nNumWaves		= r.m_nNumWaves;

		if( r.Initialized() )
			Initialize();
	}

	return *this;
}
Lake::~Lake()
{
	Uninitialize();
}

bool Lake::Initialize()
{
	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using glm::vec3;

	glex::Load();
		
	if( ! InitializeWaveData() )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize wave data", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! InitializeGeometry() )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize geometry", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! InitializeMaterial() )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize material", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! InitializeTextures() )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize textures", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! GetShader() )
	{
		AppLog::Ref().LogMsg( "%s effect not available for this geometry", __FUNCTION__ );
		Uninitialize();
		return false;
	}
	
	if( ! InitializeVbo( m_Plane ) )
	{
		AppLog::Ref().LogMsg( "%s failed to Create a vertex buffer from supplied geometry", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! InitializeVao() )
	{
		AppLog::Ref().LogMsg( "%s failed to Create a vertex array object", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	m_bInitialized = true;

	return m_bInitialized;
}
void Lake::Uninitialize()
{
	m_bInitialized = false;

	// geometry
	m_Plane.Uninitialize();
	m_Quad.Uninitialize();
	
	// material (not necessary)

	// textures
	m_AlphaMap.Uninitialize();
	m_TextureMap.Uninitialize();

	// Shader
	m_pEffect = NULL;

	if( m_pVbo )
	{
		delete m_pVbo;
		m_pVbo = NULL;
	}

	m_sAlphaMap.clear();
	m_sNormalMap.clear();
}
	
bool Lake::InitializeGeometry()
{
	// geometry
	
	using AntiMatter::AppLog;

	float rCellWidth	= 5.0f;
	float rCellHeight	= 5.0f;
	float rCellDepth	= 0.1f;

	m_Plane = Plane( 
		(int)(m_rWidth/rCellWidth), (int)(m_rHeight/rCellHeight),
		rCellWidth, 
		rCellHeight, 
		rCellDepth 
	);

	m_Quad = Quad( m_rWidth, m_rHeight );

	if( ! m_Plane.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed to initialize plane geometry", __FUNCTION__ );
		return false;
	}
	
	if( ! m_Quad.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed to initialize quad geometry", __FUNCTION__ );
		return false;
	}

	return true;
}
bool Lake::InitializeMaterial()
{
	// Material

	using glm::vec3;

	m_material.Ka(vec3(0.1, 0.1, 0.3));
	m_material.Kd(vec3(0.1, 0.1, 0.3));
	m_material.Ks(vec3(0.1, 0.1, 0.3));
	m_material.Shininess( 32.0f );

	return true;
}
bool Lake::InitializeTextures()
{
	// Textures

	using AntiMatter::Shell::FileExists;
	using AntiMatter::AppLog;

	if( FileExists( m_sTextureMap ) )
		m_TextureMap = Texture( m_sTextureMap );

	if( FileExists(m_sAlphaMap) )
		m_AlphaMap = Texture( m_sAlphaMap );

	if( ! m_TextureMap.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed to load texture map", __FUNCTION__);
		Uninitialize();
		return false;
	}			
	
	if( ! m_AlphaMap.Initialized() )
	{
		AppLog::Ref().LogMsg( "%s failed to load alpha map", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	return true;
}
bool Lake::InitializeVbo( IGeometry & geometry )
{
	using AntiMatter::AppLog;

	m_pVbo = new Vbo<CustomVertex> ( 
		geometry.VertCount(), 
		geometry.Vertices(), 
		geometry.IndexCount(), 
		geometry.Indices() 
	);

	if( ! m_pVbo->Initialized() )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize vertex buffer for object geometry", __FUNCTION__ );
		return false;
	}

	return true;
}
bool Lake::InitializeVao()
{	
	using AntiMatter::AppLog;

	glUseProgram( m_pEffect->Id() );

	glGenVertexArrays( 1, &m_nVaoId );
	glBindVertexArray( m_nVaoId );	

	glBindBuffer( GL_ARRAY_BUFFER,			m_pVbo->Id() );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	m_pVbo->IndexId() );
	
	// set the VBO attribute pointers
	GLuint i = 0;
	
	for( auto n = m_pEffect->Attributes().begin(); n != m_pEffect->Attributes().end(); n ++ )
	{	
		glEnableVertexAttribArray(i);
		glVertexAttribPointer( 
			i, 
			n->nFieldSize,
			GL_FLOAT,
			GL_FALSE,
			n->nStride,
			(GLfloat *) NULL + n->nFieldOffset
		);
		
		i ++;
	}

	glBindTexture( GL_TEXTURE0, m_TextureMap.TextureId() );
	glBindTexture( GL_TEXTURE1, m_AlphaMap.TextureId() );
	
	glBindVertexArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindTexture( GL_TEXTURE0, 0 );
	glBindTexture( GL_TEXTURE1, 0 );
	

	glUseProgram(0);

	return true;
}
bool Lake::InitializeWaveData()
{
	if( m_nNumWaves <= 0 )
		return false;

	using AntiMatter::g_Pi;	
	using std::mt19937;
	using std::uniform_real_distribution;

	typedef mt19937										Engine;
	typedef uniform_real_distribution<float>			Distribution;
	
	auto r = std::bind(Distribution(0.0f, 1.0f), Engine((DWORD)time(NULL)));

	m_rAmplitude.resize(m_nNumWaves);
	m_rWavelength.resize(m_nNumWaves);
	m_rVelocity.resize(m_nNumWaves);
	m_vDirection.resize(m_nNumWaves);

	for( int n = 0; n < m_nNumWaves; n ++ )
	{
		float theta			= glm::mix( 0.0f, 2.0f * g_Pi, r() );

		m_rAmplitude[n]		= 0.75f/(n+1);
		m_rWavelength[n]	= 13.77f * g_Pi / (n+1);
		m_rVelocity[n]		= 3.59f * n;
		m_vDirection[n]		= glm::vec2( sinf(theta), -cosf(theta) );
	}

	return true;
}
bool Lake::GetShader()
{
	using std::string;

// warning C4482: nonstandard extension used: enum 'Effect::EffectBuildState' used in qualified name
#pragma warning (push)
#pragma warning (disable: 4482)
	if( EffectMgr::Ref().Find( string("lake"), &m_pEffect ) )
	{
		if( m_pEffect->BuildState() == Effect::EffectBuildState::Linked )
		{
			m_nSunSub	= glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Sunlight" );
			m_nSpotsSub = glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Spotlights" );
			return true;
		}
	}
#pragma warning (pop)

	return false;
}
void Lake::SetUniforms()
{
	// Assign uniform variables

	if( !  m_pEffect )
		return;

	using namespace glm;
	using namespace AntiMatter;
	using namespace std;

	// Indicate which ShaderProgram to use
	glUseProgram( m_pEffect->Id() );

	// indicate which lighting subroutine to use in the shader
	if( m_pGraph->Lights().GetLightsState() == SceneLights::LightsState::sun )
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSunSub );
	else
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSpotsSub );
	
	// uniforms
	m_pEffect->AssignUniformFloat(	string("rSimTime"),			SeasonalTimeline::Ref().SeasonTimeline() );	
	m_pEffect->AssignUniformInt(	string("nNumLights"),		4 );

	m_pEffect->AssignUniformInt(	string("nNumWaves"),		m_nNumWaves );
	m_pEffect->AssignUniformFloat(	string("rWaterHeight"),		m_rHeight );

	// vViewPosition (camera position transformed into view space)
	vec3 vCamPos = vec3(Graph()->Cam().V() * m_Data.W() * vec4( Graph()->Cam().Pos(), 1.0));
	m_pEffect->AssignUniformVec3( string("vViewPosition"), vCamPos );

	// matrices	
	glm::mat4 mModelView	= m_pGraph->Cam().V() * m_Data.W();

	glm::mat3 mNormal(mModelView);	
	mNormal = glm::transpose(mNormal._inverse());

	m_pEffect->AssignUniformMat4( string("mModelView"),		mModelView );
	m_pEffect->AssignUniformMat3( string("mNormal"),		mNormal );
	m_pEffect->AssignUniformMat4( string("mMVP"),			m_Data.MVP() );

	// textures	
	m_pEffect->AssignUniformSampler2D( string("tex"),		m_TextureMap.TextureId() );
	m_pEffect->AssignUniformSampler2D( string("texAlpha"),	m_AlphaMap.TextureId() );		

	// Material
	m_pEffect->AssignUniformVec3(  string("material.Ka"),			m_material.Ka() );
	m_pEffect->AssignUniformVec3(  string("material.Kd"),			m_material.Kd() );
	m_pEffect->AssignUniformVec3(  string("material.Ks"),			m_material.Ks() );
	m_pEffect->AssignUniformFloat( string("material.rShininess"),	m_material.Shininess() );

	AppLog::Ref().OutputGlErrors();	

	// wave values
	for( int v = 0; v < m_nNumWaves; ++ v )
	{		
		stringstream ssAmplitude;
		ssAmplitude << "rAmplitude[" << v << "]";
		m_pEffect->AssignUniformFloat( ssAmplitude.str(), m_rAmplitude[v] );

		stringstream ssWavelength;
		ssWavelength << "rWavelength[" << v << "]";
		m_pEffect->AssignUniformFloat( ssWavelength.str(), m_rWavelength[v] );

		stringstream ssVelocity;
		ssVelocity << "rVelocity[" << v << "]";
		m_pEffect->AssignUniformFloat( ssVelocity.str(), m_rVelocity[v] );
				
		stringstream ssDirection;
		ssDirection << "vDirection[" << v << "]";
		m_pEffect->AssignUniformVec2( ssDirection.str(), m_vDirection[v] );
	}
	
	// lights - light[0-3] = spotlights, light[4] = sun
	const vector<Light*> & lights = this->Graph()->Lights().Lights();
	for( unsigned int x = 0; x < lights.size(); x++ )
	{
		// light position (transformed to view space)
		glm::vec4	vLightPos	= Graph()->Cam().V() * lights[x]->Pos();
		glm::vec3	la			= lights[x]->La();
		glm::vec3	ld			= lights[x]->Ld();
		glm::vec3	ls			= lights[x]->Ls();
		glm::vec3	vDir		= lights[x]->Direction();
		float		rExp		= lights[x]->Exponent();
		float		rCutOff		= lights[x]->CutOff();

		stringstream ssP, ssLa, ssLd, ssLs, ssD, ssE, ssC;
		ssP  << "lights[" << x << "].Position";
		ssLa << "lights[" << x << "].La";
		ssLd << "lights[" << x << "].Ld";
		ssLs << "lights[" << x << "].Ls";
		ssD  << "lights[" << x << "].vDirection";
		ssE  << "lights[" << x << "].rExponent";
		ssC  << "lights[" << x << "].rCutOff";

		string sPosition	= ssP.str();
		string sLa			= ssLa.str();
		string sLd			= ssLd.str();
		string sLs			= ssLs.str();
		string sDir			= ssD.str();
		string sExp			= ssE.str();
		string sCut			= ssC.str();

		m_pEffect->AssignUniformVec4( sPosition, vLightPos );
		m_pEffect->AssignUniformVec3( sLa, la );
		m_pEffect->AssignUniformVec3( sLd, ld );
		m_pEffect->AssignUniformVec3( sLs, ls );
		m_pEffect->AssignUniformVec3( sDir, vDir );
		m_pEffect->AssignUniformFloat( sExp, rExp );
		m_pEffect->AssignUniformFloat( sCut, rCutOff );
	}
	
	AppLog::Ref().OutputGlErrors();

}

// IGraphNode
HRESULT Lake::Update( const float & rSecsDelta )
{
	if( ! m_bInitialized )
		return S_OK;

	m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();
		
	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() *
						m_Data.Stack();
	
	// Updates child nodes
	if( IGraphNode::HasChildNodes() )
		IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Lake::PreRender()
{
	using AntiMatter::AppLog;

	if( ! m_bInitialized )
		return E_UNEXPECTED;	

	//GLuint nSunSub			= glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Sunlight" );
	//GLuint nSpotlightSub	= glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Spotlights" );

	glUseProgram( m_pEffect->Id() );

	glBindVertexArray( m_nVaoId );	

	SetUniforms();

	// execute the shader program
	// glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );	

	return S_OK;
}
HRESULT Lake::Render()
{
	using AntiMatter::AppLog;

	if( ! m_bInitialized )
		return E_UNEXPECTED;

	glDrawElements( GL_TRIANGLES, m_Plane.IndexCount(), GL_UNSIGNED_SHORT, ((char*)NULL) );
	AppLog::Ref().OutputGlErrors();

	return S_OK;
}
HRESULT Lake::PostRender()
{
	using AntiMatter::AppLog;	

	if( ! m_bInitialized )
		return E_UNEXPECTED;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	AppLog::Ref().OutputGlErrors();
	
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
	

	AppLog::Ref().OutputGlErrors();

	return S_OK;
}
HRESULT Lake::DrawItem()
{
	if( ! m_pEffect )
		return S_OK;

	// execute default behaviour, which renders all child nodes first
	IGraphNode::DrawItem();

	// Render
	PreRender();
	Render();
	PostRender();

	return S_OK;
}
