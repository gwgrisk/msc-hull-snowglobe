
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
#include "TreeShaderz.h"
#include "Material.h"

#include "slowglobe-cfg.h"

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
	m_CurrentShader				( WireFrame ),
	m_pEffect					( NULL ),
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
	m_CurrentShader				( WireFrame ),
	m_pEffect					( NULL ),
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

	if( ! GetShader( WireFrame ) )
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
	_ShaderNames.push_back(std::string("smooth-textured"));
	_ShaderNames.push_back(std::string("bump-textured"));
}
bool Tree::InitializeMtl()
{
	using glm::vec3;

	m_material.Ka( vec3(0.3, 0.3, 0.3) );
	m_material.Kd( vec3(0.7, 0.7, 0.7) );
	m_material.Ks( vec3(0.0, 0.0, 0.0) );
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
	// lights
	vector<Light*> lights = this->Graph()->Lights().Lights();
	for( unsigned int x = 0; x < lights.size(); x++ )
	{
		vec4 lightPos	= lights[x]->Pos();
		vec3 lightInt	= lights[x]->Intensity();
		
		vec4 eyeLightPos = this->Graph()->Cam().V() * lightPos;

		stringstream ssP, ssI;
			
		ssP  << "lights[" << x << "].Position";
		ssI << "lights[" << x << "].Intensity";		

		string sPosition	= ssP.str();
		string sIntensity	= ssI.str();		

		m_pEffect->AssignUniformVec4( sPosition,	 eyeLightPos );
		AppLog::Ref().OutputGlErrors();	
		m_pEffect->AssignUniformVec3( sIntensity, lightInt );
		AppLog::Ref().OutputGlErrors();	
	}	

	// textures
	m_pEffect->AssignUniformSampler2D( string("tex"),		m_texBark.TextureId() );	
	m_pEffect->AssignUniformSampler2D( string("texBump"),	m_texBump.TextureId() );	

	// Material
	m_pEffect->AssignUniformVec3(  string("Ka"),			m_material.Ka() );
	m_pEffect->AssignUniformVec3(  string("Kd"),			m_material.Kd() );
	m_pEffect->AssignUniformVec3(  string("Ks"),			m_material.Ks() );
	m_pEffect->AssignUniformFloat( string("rShininess"),	m_material.Shininess() );	
	
	// set the colour for the branch being drawn (for debug purposes)
	vec4 vColor[2] = { vec4(1.0, 0.0, 1.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0) };
	m_pEffect->AssignUniformVec4( string("wfColour"),	vColor[nFlipIt] );
	AppLog::Ref().OutputGlErrors();

	glPolygonMode( GL_FRONT_AND_BACK, ( m_CurrentShader == WireFrame ) ? GL_LINE : GL_FILL );
	
	return true;
}

bool Tree::InitializeLSystem()
{
	using std::vector;
	using std::string;
	using glm::vec3;

	// Initialize the LSystem from an existing config file.
	m_LSystem = LSystem( g_Cfg.LSystemCfg() );

	// If no config file found for the LSystem, attempt to create one
	// and initialize the LSystem from a hardcoded ruleset
	if( ! m_LSystem.Initialized() )
	{
		vector<string> vsRules;
		vsRules.push_back( string("FF--[2]+F-FF-FF") );
		vsRules.push_back( string("[3]-[3]+[3]") );
		vsRules.push_back( string("FF+F") );

		m_LSystem = LSystem( 6, 10.0f, 20.0f, vec3(0.0, 50.0, 0.0), vsRules );

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


HRESULT Tree::Update( const float & rSecsDelta )
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
HRESULT Tree::PreRender()
{
	using AntiMatter::AppLog;

	AppLog::Ref().OutputGlErrors();

	if( ! m_pEffect )
		return E_POINTER;	

	SetShaderArgs();	

	glPolygonMode( GL_FRONT_AND_BACK, ( m_CurrentShader == WireFrame ) ? GL_LINE : GL_FILL );

	return S_OK;
}
HRESULT Tree::Render()
{
	// iterate each generation of the LTree and then scale and position the cylinder, then draw it
	// for each iteration of the tree

	using glm::mat4;
	using glm::mat3;
	using glm::vec3;
	using std::string;
	using std::for_each;
	using AntiMatter::AppLog;

	int nGeneration = 0;
	int nSegment	= 0;

	nFlipIt = 0;

	for_each( m_LTree.begin(), m_LTree.end(),
		[&]( Generation & gen )
		{
			nSegment	= 0;			

			for_each( gen.begin(), gen.end(),
				[&]( Segment* pSeg )
				{
					const Segment* pParent = pSeg->Parent();
					vec3 vPos;
					vec3 vTipPos;
					real rLength;

					// position and length of segment
					if( pSeg->Generation()  > 0 )
					{
						vPos		= pParent->TipPos();
						rLength		= pSeg->Scale() * m_Cylinder.Length();
					}
					else
					{
						mat4 m		= m_Data.W();
						vPos		= vec3( m[3][0], m[3][1], m[3][2] );
						rLength		= pSeg->Scale() * m_Cylinder.Length();
					}

					pSeg->Pos( vPos );
					pSeg->Length( rLength );

					// m[] = T * R * S
					glm::mat4 mW(1.0f);
					real rAngle = (nGeneration % 2 == 0 ? 30.0f : 20.0f);
					real rX = -90.0f + rAngle * (nGeneration+1);
					real rZ = rAngle * (nSegment);

					// T
					if( pSeg->Generation() > 0 )
						mW = glm::translate( mat4(1.0f), pParent->TipPos() );
					else
						mW = glm::translate( mat4(1.0f), pSeg->Pos() );

					
					if( pSeg->Generation() > 0 )
					{
						// R
						mW *= glm::rotate( mat4(1.0f), rX, vec3(1, 0, 0) );
						mW *= glm::rotate( mat4(1.0f), rZ, vec3(0, 0, 1) );
					}
					
					// S
					mW *= glm::scale( mat4(1.0f), glm::vec3(pSeg->Scale()) );

					// orientation mtx
					pSeg->OrientationMtx( mat4(mat3(mW)) );

					// orientation vector
					glm::vec4 vO = mW * glm::vec4(0, 1, 0, 0);
					pSeg->Orientation(glm::normalize(vec3(vO)));
					
					// tip pos
					vTipPos	= vPos + (pSeg->Orientation() * pSeg->Length());
					pSeg->TipPos( vTipPos );
					
					// draw the branch					
					if( m_pEffect )
					{
						// matrices
						mat4 mModelView = m_pGraph->Cam().V() * m_Data.Stack() * mW;
						mat4 mMVP		= m_pGraph->Proj().P() * m_pGraph->Cam().V() * m_Data.Stack() * mW;

						mat3 mNormal(mModelView);
						mNormal = glm::transpose(mNormal._inverse());

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

					nFlipIt		= 1-nFlipIt;
					nSegment ++;
				}
			);
						
			nGeneration ++;
		}
	);

	return S_OK;
}
HRESULT Tree::PostRender()
{
	using AntiMatter::AppLog;

	AppLog::Ref().OutputGlErrors();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
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

glm::mat4 Tree::CalcSegOrientationMatrix( const glm::vec3 & vOrientation, const glm::vec3 & vPos  )
{
	// I'm attempting to create a rotation matrix that represents the 
	// vOrientation matrix by splitting orientation vec3 into two 
	// rotations.  rotation about the x and z axes respectively.

	// here's the transformation matrix
	// |cosθ cosφ   -sinθ   -cosθ sinφ|
	// |sinθ cosφ    cosθ   -sinθ sinφ|
	// |   sinφ       0         cosφ  |

	UNREFERENCED_PARAMETER(vPos);
    
	const glm::vec3 & v = vOrientation;

	/* Find cosφ and sinφ */
    float c1 = sqrt(v.x * v.x + v.y * v.y);
    float s1 = v.z;

    /* Find cosθ and sinθ; if gimbal lock, choose (1,0) arbitrarily */
    float c2 = c1 ? v.x / c1 : 1.0f;
    float s2 = c1 ? v.y / c1 : 0.0f;
	
	return glm::mat4 (
		v.x,	-s2,	-s1*c2,		0,
		v.y,	c2,		-s1*c2,		0,
		v.z,	0,		c1,			0,
		0,		0,		0,			1
	);
	/*
	return glm::mat4 (
		v.x,	-s2,	-s1*c2,		0,
		v.y,	c2,		-s1*c2,		0,
		v.z,	0,		c1,			0,
		vPos.x,	vPos.y,	vPos.z,		1
	);
	*/
}