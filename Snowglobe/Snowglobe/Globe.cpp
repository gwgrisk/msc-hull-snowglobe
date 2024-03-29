
#include "stdafx.h"

#include "Globe.h"
#include "IGraphNode.h"
#include "IGeometry.h"
#include "SceneGraph.h"
#include "VertexClass.h"
#include "Sphere.h"

#include "EffectMgr.h"
#include "Effect.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"

#include "slowglobe-cfg.h"

#include <string>
#include <glm\glm.hpp>
#include <AntiMatter\AppLog.h>
#include <AntiMatter\ShellUtils.h>
#include <AntiMatter\constants.h>

#include <glm\glm.hpp>
#include "Simulation.h"

Globe::Globe() : 
	IGraphNode			( NULL, NULL, std::string("globe") ),
	m_pEffect			( NULL ),
	m_pVbo				( NULL ),
	m_nVaoId			( 0 ),
	m_bInitialized		( false ),		
	m_rRadius			( 10.0f ),
	m_nStacks			( 10 ),
	m_nSlices			( 10 )
{
	m_bInitialized = Initialize();
}
Globe::Globe(	SceneGraph * pGraph, IGraphNode* pParent, const std::string & sId, 
				float rRadius, int nSlices, int nStacks, 
				const std::string & sTexture,
				const glm::vec4 & vColour ) : 
	IGraphNode			( pGraph, pParent, sId ),
	m_pEffect			( NULL ),
	m_pVbo				( NULL ),
	m_nVaoId			( 0 ),
	m_bInitialized		( false ),
	m_sTexture			( sTexture ),
	m_rRadius			( rRadius ),
	m_nStacks			( nStacks ),
	m_nSlices			( nSlices ),
	m_vColour			( vColour )
{
	m_bInitialized = Initialize();
}
Globe::~Globe()
{
	try	
	{
		Uninitialize();
	}
	catch(...)
	{
	}	
}
	

bool Globe::Initialize()
{
	using namespace AntiMatter;
	using glm::vec3;

	glex::Load();

	if( m_bInitialized )
		return true;
		
	InitializeMaterial();

	if( ! InitializeGeometry() )
	{
		AppLog::Ref().LogMsg("%s initialize geometry failed for globe object", __FUNCTION__ );
		return false;
	}
	
	if( ! InitializeTextures() )
	{
		AppLog::Ref().LogMsg("%s initialize textures failed for globe object", __FUNCTION__ );
		return false;
	}
	
	if( ! SetPerVertexColour() )	// can only do this after the geometry has been created
	{
		AppLog::Ref().LogMsg("%s initialize vertex colours failed for globe object", __FUNCTION__ );
		return false;
	}

	if( ! GetShader() )
	{
		AppLog::Ref().LogMsg( "%s effect not available for this geometry", __FUNCTION__ );
		Uninitialize();
		return false;
	}
	
	if( ! InitializeVbo( m_Sphere ) )
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
void Globe::Uninitialize()
{
	if( ! m_bInitialized )
		return;

	m_bInitialized = false;

	// geometry	
	m_Sphere.Uninitialize();
	
	// material (not necessary)

	// textures
	m_tex.Uninitialize();
	
	// Shader
	m_pEffect = NULL;

	// vbo
	if( m_pVbo )
	{
		delete m_pVbo;
		m_pVbo = NULL;
	}

	// vao
	if( m_nVaoId )
		glDeleteVertexArrays( 1, &m_nVaoId );
}

bool Globe::InitializeGeometry()
{
	using AntiMatter::AppLog;

	m_Sphere = Sphere( m_rRadius, m_nSlices, m_nStacks );

	if( ! m_Sphere.Initialized() )
	{
		AppLog::Ref().LogMsg("Globe::Initialize() failed, geometry primitive Sphere didn't initialize properly");
		return false;
	}

	return true;
}
void Globe::InitializeMaterial()
{
	using glm::vec3;

	// material
	m_material.Ka(vec3(0.0f, 0.0f, 0.0f));
	m_material.Kd(vec3(0.2f, 0.2f, 0.2f));
	m_material.Ks(vec3(1.0, 1.0, 1.0));
	m_material.Shininess( 32.0f );
}
bool Globe::InitializeTextures()
{
	using namespace AntiMatter;

	if( m_sTexture.length() == 0 )
	{
		glBindTexture( GL_TEXTURE_2D, 0 );
		return true;
	}

	if( ! Shell::FileExists( m_sTexture ) )
	{
		glBindTexture( GL_TEXTURE_2D, 0 );
		AppLog::Ref().LogMsg("%s texture not found", __FUNCTION__ );
		return true; // yes, I really mean true!  so what if there's no texture!?
	}
	
	m_tex = Texture( m_sTexture );
	if(!  m_tex.Initialized() )
	{
		AppLog::Ref().LogMsg("Globe::Initialize() texture load failed");
		return false;
	}

	return true;
}
bool Globe::SetPerVertexColour()
{
	// copies the specified colour to each vertex
	CustomVertex* pVertices  =*(m_Sphere.Vertices());
	
	for( int n = 0; n < m_Sphere.VertCount(); n ++ )
	{
		pVertices[n].m_Colour[0] = m_vColour.r;
		pVertices[n].m_Colour[1] = m_vColour.g;
		pVertices[n].m_Colour[2] = m_vColour.b;
		pVertices[n].m_Colour[3] = m_vColour.a;
	}
	
	return true;
}

bool Globe::InitializeVbo( IGeometry & geometry )
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
bool Globe::InitializeVao()
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
	
	glBindVertexArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindTexture( GL_TEXTURE0, 0 );
	glBindTexture( GL_TEXTURE1, 0 );
	

	glUseProgram(0);

	return true;
}

bool Globe::GetShader()
{
	using std::string;

// warning C4482: nonstandard extension used: enum 'Effect::EffectBuildState' used in qualified name
#pragma warning (push)
#pragma warning (disable: 4482)
	if( EffectMgr::Ref().Find( string("globe"), &m_pEffect ) )
	{
		if( m_pEffect->BuildState() == Effect::EffectBuildState::Linked )
		{
			// Get the shader variable value that'll allow us to select which lighting subroutine
			// to use in the fragment shader code (one for the sun, one for the spotlights)
			m_nSunSub	= glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Sunlight" );
			m_nSpotsSub = glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Spotlights" );
			return true;
		}
	}
#pragma warning (pop)

	return false;
}
bool Globe::SetUniforms()
{
	using namespace AntiMatter;
	using namespace glm;
	using std::vector;
	using std::stringstream;
	using std::string;

	if( ! m_pEffect )
		return false;

	// Indicate which ShaderProgram to use
	glUseProgram( m_pEffect->Id() );
	
	// indicate which lighting subroutine to use in the shader
	if( m_pGraph->Lights().GetLightsState() == SceneLights::LightsState::sun )
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSunSub );
	else
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSpotsSub );

	// bind buffers		
	glBindVertexArray( m_nVaoId );
	glBindBuffer( GL_ARRAY_BUFFER, m_pVbo->Id() );	

	// ensure no texture selected (might've left one selected from previous object)
	glBindTexture( GL_TEXTURE_2D, 0 );	


	// Assign uniform variables	

	// vViewPosition (camera position transformed into view space)	
	vec3 vCamPos = vec3(Graph()->Cam().V() * vec4( Graph()->Cam().Pos(), 1.0));
	m_pEffect->AssignUniformVec3( string("vViewPosition"), vCamPos );
	
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

	// material data
	m_pEffect->AssignUniformVec3(  string("material.Ka"),			m_material.Ka() );
	m_pEffect->AssignUniformVec3(  string("material.Kd"),			m_material.Kd() );
	m_pEffect->AssignUniformVec3(  string("material.Ks"),			m_material.Ks() );
	m_pEffect->AssignUniformFloat( string("material.rShininess"),	m_material.Shininess() );

	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mView * mModel;

	glm::mat3 mNormal(mModelView);		
	mNormal = glm::transpose(mNormal._inverse());
	
	m_pEffect->AssignUniformMat4(	string("mModelView"),	mModelView );	
	m_pEffect->AssignUniformMat4(	string("mMVP"),			m_Data.MVP() );
	m_pEffect->AssignUniformMat3(	string("mNormal"),		mNormal );

	return true;
}


// IGraphNode
HRESULT Globe::Update( const float & rSecsDelta )
{
	m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();
	
	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() *
						m_Data.Stack();

	// Updates child nodes
	if( IGraphNode::HasChildNodes() )
		IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Globe::PreRender()
{
	// need to disable depth test for transparent objects
	// that aside, the sphere geometry is rendered as two triangle fans
	// and a series of triangle strips.

	using AntiMatter::AppLog;

	AppLog::Ref().OutputGlErrors();

	if( ! m_pEffect )
		return E_POINTER;
	
	if( m_tex.Initialized() )
		glBindTexture( GL_TEXTURE_2D, m_tex.TextureId() );
	else
		glBindTexture( GL_TEXTURE_2D, 0 );
	
	glDisable( GL_DEPTH_TEST );
	SetUniforms();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return S_OK;
}
HRESULT Globe::Render()
{
	//return S_OK;
	using AntiMatter::AppLog;

	int nSlices = m_Sphere.Slices();
	int nStacks = m_Sphere.Stacks();

	// top triangle fan
	glDrawElements( 
		GL_TRIANGLE_FAN, 
		nSlices+2,
		GL_UNSIGNED_SHORT, 
		m_Sphere.Indices()
	);

	// stacks
	for( int n = 0; n < nStacks-2; n ++ )
	{
		glDrawElements(
			GL_TRIANGLE_STRIP,
			((nSlices+1)*2),
			GL_UNSIGNED_SHORT,
			&(m_Sphere.Indices())[ nSlices+2+n*(nSlices+1)*2 ]
		);
	}

	// bottom triangle fan
	glDrawElements( 
		GL_TRIANGLE_FAN, 
		nSlices+2,
		GL_UNSIGNED_SHORT, 
		&(m_Sphere.Indices())[ nSlices+2+(nStacks-2)*(nSlices+1)*2 ]
	);

	return S_OK;
}
HRESULT Globe::PostRender()
{
	using AntiMatter::AppLog;

	AppLog::Ref().OutputGlErrors();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
	glBindVertexArray(0);
	glUseProgram(0);

	glEnable( GL_DEPTH_TEST );	


	AppLog::Ref().OutputGlErrors();
	return S_OK;
}
HRESULT Globe::DrawItem()
{
	// execute default behaviour, which renders all child nodes first
	IGraphNode::DrawItem();

	// Render terrain:
	PreRender();
	Render();
	PostRender();

	return S_OK;
}