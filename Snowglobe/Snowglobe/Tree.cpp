﻿
#include "stdafx.h"

#include "Tree.h"
#include "VertexClass.h"
#include "SceneGraph.h"

#include "Vbo.h"
#include "VertexClass.h"
#include "EffectMgr.h"
#include "Effect.h"
#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"

#include "Segment.h"
#include "LSystem.h"
#include "Cylinder.h"
#include "Material.h"

#include "slowglobe-cfg.h"
#include "InputMgr.h"

#include <GL\GL.h>
#include <GXBase.h>
#include <AntiMatter\AppLog.h>
#include <glm\glm.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <comdef.h>
#include <algorithm>

static int nFlipIt = 1;

Tree::Tree() :
	m_bInitialized				( false ),
	m_CurrentShader				( Smooth ),
	m_pEffect					( NULL ),
	m_nSunSub					( 0 ),
	m_nSpotlightSub				( 0 ),
	m_pVbo						( NULL ),
	m_nVaoId					( 0 ),
	m_rInitialSegLength			( 400.0f )
{
	
}
Tree::Tree( SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId,
			const std::string & sBark, const std::string & sBump ) :

	IGraphNode					( pGraph, pParent, sId ),
	m_bInitialized				( false ),	
	m_sBarkFile					( sBark ),
	m_sBumpFile					( sBump ),
	m_CurrentShader				( Smooth ),
	m_pEffect					( NULL ),
	m_nSunSub					( 0 ),
	m_nSpotlightSub				( 0 ),
	m_pVbo						( NULL ),
	m_nVaoId					( 0 ),
	m_rInitialSegLength			( 400.0f )
{
	m_bInitialized = Initialize();
}
Tree::Tree( const Tree & r ) :
	IGraphNode					( r.m_pGraph, r.m_pParent, r.m_sId ),
	m_bInitialized				( false ),
	m_sBarkFile					( r.m_sBarkFile ),
	m_sBumpFile					( r.m_sBumpFile ),
	m_CurrentShader				( r.m_CurrentShader ),
	m_pEffect					( NULL ),
	m_pVbo						( NULL ),
	m_nVaoId					( 0 ),
	m_rInitialSegLength			( 400.0f )
{
	m_bInitialized = Initialize();
}
Tree & Tree::operator=( const Tree & r )
{
	if( this != &r )
	{
		this->Uninitialize();

		m_pGraph			= r.m_pGraph;
		m_pParent			= r.m_pParent;
		m_sId				= r.m_sId;

		m_sBarkFile			= r.m_sBarkFile;
		m_sBumpFile			= r.m_sBumpFile;
		m_CurrentShader		= r.m_CurrentShader;
		m_rInitialSegLength = r.m_rInitialSegLength;

		m_bInitialized	= Initialize();
	}

	return *this;
}
Tree::~Tree()
{
	Uninitialize();
}

bool Tree::Initialize()
{	
	// TODO: If any init step fails, then the object might be left half initialized
	// fix this with a call to Uninitialize()!!!
	using std::vector;
	using std::string;
	using AntiMatter::AppLog;
	using glm::vec3;
	
	InitShaderNames();
	InitializeMtl();	

	if( ! InitializeTextures() )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to load textures", __FUNCTION__);
		return false;
	}

	if( ! InitializeGeometry() )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s initialize geometry failed for cylinder", __FUNCTION__ );
		return false;
	}

	if( ! InitializePerVertexColour( glm::vec4(0.4f, 0.2f, 0.2f, 1.0f) ) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s initialize per vertex colour failed for cylinder", __FUNCTION__ );
		return false;
	}

	if( ! GetShader( m_CurrentShader ) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize shaders for the tree", __FUNCTION__);
		return false;
	}

	if( ! InitializeVbo(m_Cylinder) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize vbo", __FUNCTION__);
		return false;
	}

	if( ! InitializeVao() )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize vao", __FUNCTION__);
		return false;		
	}		
	
	if( ! InitializeLSystem() )
	{
		Uninitialize();
		AppLog::Ref().LogMsg( "%s failed to initialize the LSystem", __FUNCTION__ );
		return false;
	}	

	HRESULT hr = InitializeLTree();
	if( ! SUCCEEDED(hr) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg(
			"%s failed while attempting to initialize the LTree: %s",
			__FUNCTION__,
			_com_error(hr).ErrorMessage()
		);		
		return false;
	}

	return true;
}
void Tree::Uninitialize()
{
	_ShaderNames.clear();
	m_pEffect = NULL;

	if( m_pVbo )
	{
		delete m_pVbo;
		m_pVbo = NULL;	
	}

	m_nVaoId = 0;

	m_LTree.erase( m_LTree.begin(), m_LTree.end() );
	m_LSystem.Uninitialize();
	m_Cylinder.Uninitialize();

	m_rInitialSegLength = 400.0f;
}

void Tree::InitShaderNames()
{
	_ShaderNames.clear();
	_ShaderNames.reserve(5);
	_ShaderNames.push_back(std::string("wireframe"));
	_ShaderNames.push_back(std::string("flat"));
	_ShaderNames.push_back(std::string("smooth"));
	_ShaderNames.push_back(std::string("textured-phong"));
	_ShaderNames.push_back(std::string("bump-textured"));
}
bool Tree::InitializeMtl()
{
	using glm::vec3;

	m_material.Ka( vec3(0.3, 0.3, 0.3) );
	m_material.Kd( vec3(0.7, 0.7, 0.7) );
	m_material.Ks( vec3(0.2, 0.1, 0.1) );
	m_material.Shininess( 1.0f );

	return true;
}
bool Tree::InitializeGeometry()
{
	using AntiMatter::AppLog;
	// Initialize a single Cylinder (IGeometry object), which is scaled and reused to draw
	// each segment of the tree
	// m_Cylinder = Cylinder( 8, 13, 15.0f, 5.0f, 3.0f );
	m_Cylinder = Cylinder( 8, 6, 20.0f, 5.0f, 3.5f );

	if( ! m_Cylinder.Initialized() )
	{
		AppLog::Ref().LogMsg("Cylinder::Initialize() failed, geometry primitive Sphere didn't initialize properly");
		return false;
	}

	return true;
}
bool Tree::InitializeTextures()
{
	using AntiMatter::Shell::FileExists;

	// load texture
	if( FileExists(m_sBarkFile) )
		m_texBark = Texture( m_sBarkFile.c_str(), false );

	// load bump map
	if( FileExists( m_sBumpFile ) )
		m_texBump = Texture( m_sBumpFile.c_str(), false );


	return (m_texBark.Initialized() && m_texBump.Initialized());
}
bool Tree::InitializePerVertexColour( const glm::vec4 & col )
{
	// copies the specified colour to each vertex
	CustomVertex* pVertices  =*(m_Cylinder.Vertices());
	
	for( int n = 0; n < m_Cylinder.VertCount(); n ++ )
	{
		pVertices[n].m_Colour[0] = col.r;
		pVertices[n].m_Colour[1] = col.g;
		pVertices[n].m_Colour[2] = col.b;
		pVertices[n].m_Colour[3] = col.a;
	}
	
	return true;
}
	
bool Tree::InitializeVbo( IGeometry & geometry )
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
bool Tree::InitializeVao()
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
	
bool Tree::GetShader( const eTreeShader e )
{
	using std::string;

	m_CurrentShader = e;

// warning C4482: nonstandard extension used: enum 'Effect::EffectBuildState' used in qualified name
#pragma warning (push)
#pragma warning (disable: 4482)
	if( EffectMgr::Ref().Find( _ShaderNames[e], &m_pEffect ) )
	{
		if( m_pEffect->BuildState() == Effect::EffectBuildState::Linked )
		{
			// Get the shader variable value that'll allow us to select which lighting subroutine
			// to use in the fragment shader code (one for the sun, one for the spotlights)
			// m_nSunSub	= glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Sunlight" );
			// m_nSpotsSub = glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Spotlights" );
			return true;
		}
	}
#pragma warning (pop)

	return false;
}

bool Tree::SetShaderArgs()
{
	using namespace glm;
	using namespace std;
	using namespace AntiMatter;

	if( ! m_pEffect )
		return false;
	
	// Select Shader
	glUseProgram( m_pEffect->Id() );	

	// bind buffers
	glBindVertexArray( m_nVaoId );
	glBindBuffer( GL_ARRAY_BUFFER,			m_pVbo->Id() );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	m_pVbo->IndexId() );
	glBindTexture( GL_TEXTURE_2D,			m_texBark.TextureId() );
	glBindTexture( GL_TEXTURE_2D,			m_texBump.TextureId() );
	
	// Assign uniform variables

	// camera position (transformed to view space)
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

	// Material
	m_pEffect->AssignUniformVec3(  string("material.Ka"),			m_material.Ka() );
	m_pEffect->AssignUniformVec3(  string("material.Kd"),			m_material.Kd() );
	m_pEffect->AssignUniformVec3(  string("material.Ks"),			m_material.Ks() );
	m_pEffect->AssignUniformFloat( string("material.rShininess"),	m_material.Shininess() );

	// textures
	if( m_CurrentShader != eTreeShader::WireFrame )
	{
		m_pEffect->AssignUniformSampler2D( string("tex"),		m_texBark.TextureId() );
		m_pEffect->AssignUniformSampler2D( string("texBump"),	m_texBump.TextureId() );
	}
	
	// wireframe branch colour
	if( m_CurrentShader == eTreeShader::WireFrame )
	{
		vec4 vColor[2] = { vec4(1.0, 0.0, 1.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0) };
		m_pEffect->AssignUniformVec4( string("wfColour"),	vColor[nFlipIt] );		
	}

	switch( m_CurrentShader )
	{
	case eTreeShader::Flat:
		SelectShaderSubroutine( GL_VERTEX_SHADER );
		break;

	default:
		SelectShaderSubroutine( GL_FRAGMENT_SHADER );
		break;
	};
	
	glPolygonMode( GL_FRONT_AND_BACK, ( m_CurrentShader == WireFrame ) ? GL_LINE : GL_FILL );
	
	AppLog::Ref().OutputGlErrors();
	return true;
}
void Tree::SelectShaderSubroutine( GLenum shadertype )
{
	m_nSunSub		= glGetSubroutineIndex( m_pEffect->Id(), shadertype, "Sunlight" );
	m_nSpotlightSub = glGetSubroutineIndex( m_pEffect->Id(), shadertype, "Spotlights" );

	if( m_pGraph->Lights().GetLightsState() == SceneLights::LightsState::sun )
		glUniformSubroutinesuiv( shadertype, 1, &m_nSunSub );
	else
		glUniformSubroutinesuiv( shadertype, 1, &m_nSpotlightSub );
}

bool Tree::InitializeLSystem()
{
	using std::vector;
	using std::string;
	using glm::vec3;

	// Initialize the LSystem from an existing config file.
	m_LSystem = LSystem( this, g_Cfg.LSystemCfg() );

	// If no config file found for the LSystem, attempt to create one
	// and initialize the LSystem from a hardcoded ruleset
	if( ! m_LSystem.Initialized() )
	{
		vector<string> vsRules;
		vsRules.push_back( string("FF--[2]+F-FF-FF") );
		vsRules.push_back( string("[3]-[3]+[3]") );
		vsRules.push_back( string("FF+F") );

		m_LSystem = LSystem( this, 6, 10.0f, 20.0f, vec3(0.0, 50.0, 0.0), vsRules );

		if( m_LSystem.Initialized() )
			m_LSystem.PersistSet( g_Cfg.LSystemCfg() );
	}

	return m_LSystem.Initialized();
}
HRESULT Tree::InitializeLTree()
{
	using glm::mat4;
	
	if( m_bInitialized )
		return S_OK;

	if( ! m_LSystem.Initialized() )
		return E_UNEXPECTED;

	Segment * pNextSeg = m_LSystem.LSystemData();
	
	if( ! pNextSeg )
		return E_POINTER;

	m_LTree.resize( m_LSystem.MaxGenerations() + 1 );
	
	AddSegChildren( pNextSeg );
	
	return S_OK;
}
void Tree::AddSegChildren( Segment* pSeg )
{
	if( ! pSeg )
		return;

	Segment::ChildSegments & cs = pSeg->ChildSegs();

	if( cs.size() > 0 )
	{
		std::for_each( cs.begin(), cs.end(),

			[&]( Segment* pNext )
			{
				AddSegChildren(pNext);
				AddSegment(pNext);
			}
		);		

		AddSegment(pSeg);
	}
	else
	{
		AddSegment(pSeg);
	}
}
void Tree::AddSegment( Segment* pSeg )
{
	if( ! pSeg )
		return;

	int nGeneration;
	
	nGeneration = pSeg->Generation();

	m_LTree[nGeneration].push_back(pSeg);
}

void Tree::RenderSegment( Segment* pSeg )
{
	using glm::mat4;
	using glm::mat3;
	using glm::transpose;
	using glm::inverse;
	using std::string;

	if( ! m_pEffect || ! pSeg )
		return;
	
	mat4 mModelView = m_pGraph->Cam().V() * m_Data.Stack() * pSeg->mW();
	mat4 mMVP		= m_pGraph->Proj().P() * m_pGraph->Cam().V() * m_Data.Stack() * pSeg->mW();

	mat3 mNormal(mModelView);
	mNormal = transpose(mNormal._inverse());

	m_pEffect->AssignUniformMat4( string("mModelView"),		mModelView );
	m_pEffect->AssignUniformMat3( string("mNormal"),		mNormal );
	m_pEffect->AssignUniformMat4( string("mMVP"),			mMVP );
				
	glDrawElements(
		GL_TRIANGLE_STRIP,
		(((m_Cylinder.Slices()+1) * 2) * m_Cylinder.Stacks()),
		GL_UNSIGNED_SHORT,
		(GLvoid*)m_Cylinder.Indices()[0]
	);
}

HRESULT Tree::Update( const float & rSecsDelta )
{
	if( rSecsDelta > 1.0f )
		return S_OK;

	m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();
	
	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() *
						m_Data.Stack();	

	// Updates child nodes
	if( IGraphNode::HasChildNodes() )
		IGraphNode::UpdateChildren( rSecsDelta );

	// handle keyboard input for changing currently selected tree shader
	if( InputMgr::Ref().Keybd().F12() == KeyData::KeyDown )
	{		
		int nShader = (int) m_CurrentShader;
		
		nShader ++;

		if( nShader > BumpTextured )
			nShader = WireFrame;

		if(! GetShader((eTreeShader)nShader) )
			GetShader( WireFrame );

		InputMgr::Ref().Keybd().F12(KeyData::KeyUp);
	}
	
	return S_OK;
}
HRESULT Tree::PreRender()
{
	using AntiMatter::AppLog;

	AppLog::Ref().OutputGlErrors();

	if( ! m_pEffect )
		return E_POINTER;

	SetShaderArgs();	

	switch( m_CurrentShader )
	{
	case WireFrame:
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		break;

	case Flat:		
		glShadeModel(GL_FLAT);
		glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
		break;

	case Smooth:
		break;

	case SmoothTextured:
		break;

	case BumpTextured:
		break;

	}

	return S_OK;
}
HRESULT Tree::Render()
{
	// iterate each generation of the LTree and 
	// update and render each segment

	using std::for_each;	
	
	nFlipIt = 0;

	for_each( m_LTree.begin(), m_LTree.end(),
		[&]( Generation & gen )
		{
			for_each( gen.begin(), gen.end(),
				[&]( Segment* pSeg )
				{
					pSeg->Update(0);
					RenderSegment(pSeg);
					nFlipIt	= 1-nFlipIt;
				}
			);
		}
	);

	return S_OK;
}
HRESULT Tree::PostRender()
{
	using AntiMatter::AppLog;

	AppLog::Ref().OutputGlErrors();

	
	glShadeModel( GL_SMOOTH );
	glPolygonMode(	GL_FRONT_AND_BACK,			GL_FILL);
	glBindTexture(	GL_TEXTURE_2D,				0 );
	glBindBuffer(	GL_ARRAY_BUFFER,			0 );
	glBindBuffer(	GL_ELEMENT_ARRAY_BUFFER,	0 );

	glBindVertexArray(0);
	glUseProgram(0);

	AppLog::Ref().OutputGlErrors();
	return S_OK;
}
HRESULT Tree::DrawItem()
{
	// default behaviour draws child nodes first
	IGraphNode::DrawItem();

	PreRender();
	Render();
	PostRender();

	return S_OK;
}
