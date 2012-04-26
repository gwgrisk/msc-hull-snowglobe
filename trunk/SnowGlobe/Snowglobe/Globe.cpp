
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

	if( m_pVbo )
	{
		delete m_pVbo;
		m_pVbo = NULL;
	}
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
	m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
	m_material.Kd(vec3(0.2f, 0.2f, 0.2f));
	m_material.Ks(vec3(0.8f, 0.8f, 0.8f));
	m_material.Shininess( 4.0f );	
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
	// copies the specified colour to each (and every!) vertex!!!
	// is there _any_ point to this?!
	CustomVertex* pVertices  =*(m_Sphere.Vertices());
	
	for( int n = 0; n < m_Sphere.VertCount(); n ++ )
	{
		pVertices[n].m_colour[0] = m_vColour.r;
		pVertices[n].m_colour[1] = m_vColour.g;
		pVertices[n].m_colour[2] = m_vColour.b;
		pVertices[n].m_colour[3] = m_vColour.a;
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
			return true;
	}
#pragma warning (pop)

	return false;
}
bool Globe::SetUniforms()
{
	using namespace AntiMatter;
	using namespace glm;
	using std::string;

	if( ! m_pEffect )
		return false;

	// Indicate which ShaderProgram to use
	glUseProgram( m_pEffect->Id() );

	// bind buffers		
	glBindVertexArray( m_nVaoId );
	glBindBuffer( GL_ARRAY_BUFFER, m_pVbo->Id() );	

	// ensure no texture selected (might've left one selected from previous object)
	glBindTexture( GL_TEXTURE_2D, 0 );	

	// Assign uniform variables	
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mView * mModel;

	glm::mat3 mNormal(mModelView);		
	mNormal = glm::transpose(mNormal._inverse());
	
	m_pEffect->AssignUniformFloat(	string("rFallOff"),		0.995f );
	m_pEffect->AssignUniformMat4(	string("mModelView"),	mModelView );	
	m_pEffect->AssignUniformMat4(	string("mMVP"),			m_Data.MVP() );
	m_pEffect->AssignUniformMat3(	string("mNormal"),		mNormal );

	AppLog::Ref().OutputGlErrors();

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
	IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Globe::PreRender()
{
	// need to disable depth test for transparent objects
	// that aside, the sphere geometry is rendered as two triangle fans
	// and a series of triangle strips.

	if( ! m_pEffect )
		return E_POINTER;

	using namespace AntiMatter;
	
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
		GL_UNSIGNED_INT, 
		m_Sphere.Indices()
	);

	// stacks
	for( int n = 0; n < nStacks-2; n ++ )
	{
		glDrawElements(
			GL_TRIANGLE_STRIP,
			((nSlices+1)*2),
			GL_UNSIGNED_INT,
			&(m_Sphere.Indices())[ nSlices+2+n*(nSlices+1)*2 ]
		);
	}

	// bottom triangle fan
	glDrawElements( 
		GL_TRIANGLE_FAN, 
		nSlices+2,
		GL_UNSIGNED_INT, 
		&(m_Sphere.Indices())[ nSlices+2+(nStacks-2)*(nSlices+1)*2 ]
	);

	AppLog::Ref().OutputGlErrors();

	return S_OK;
}
HRESULT Globe::PostRender()
{
	using AntiMatter::AppLog;

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