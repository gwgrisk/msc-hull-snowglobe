
#include "stdafx.h"

#include "Tree.h"
#include "VertexClass.h"
#include "SceneGraph.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "ShaderProgram.h"

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
#include <comdef.h>


Tree::Tree() :
	m_bInitialized				( false ),
	m_CurrentShader				( WireFrame ),
	m_pCurrentShader			( NULL ),
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
	m_pCurrentShader			( NULL ),
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
	m_pCurrentShader			( NULL ),
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
	using std::vector;
	using std::string;
	using AntiMatter::AppLog;
	using glm::vec3;

	// Initialize a single Cylinder (IGeometry object), which is scaled and reused to draw
	// each segment of the tree
	m_Cylinder = Cylinder( 8, 13, 15.0f, 5.0f, 3.0f );

	m_material.Ka( vec3(0.3, 0.3, 0.3) );
	m_material.Kd( vec3(0.7, 0.7, 0.7) );
	m_material.Ks( vec3(0.0, 0.0, 0.0) );
	m_material.Shininess( 1.0f );


	if( ! InitializeLSystem() )
	{
		AppLog::Ref().LogMsg( "Tree::Initialize() failed to initialize the LSystem"	);

		Uninitialize();
		return false;
	}

	HRESULT hr;
	
	hr = InitializeLTree();
	if( ! SUCCEEDED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Tree::Initialize() failed while attempting to initialize the LTree: %s",
			_com_error(hr).ErrorMessage()
		);

		Uninitialize();
		return false;
	}	

	// load texture and bump maps for each of the cylinders
	LoadTexture();
	LoadBumpMap();


	// Create the ShaderProgram objects
	InitializeShaders();

	return true;
}
void Tree::Uninitialize()
{
	UninitializeShaders();
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

		m_LSystem = LSystem( 12, 10.0f, 15.0f, vec3(0.0, 50.0, 0.0), vsRules );

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
	if( pSeg->ChildSegs().size() > 0 )
	{
		for( ChildSegments::iterator x = pSeg->ChildSegs().begin(); x != pSeg->ChildSegs().end(); x ++ )
		{
			Segment* pNext = *x;
			AddSegChildren(pNext);
			AddSegment(pNext);
		}
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

bool Tree::SetPerVertexColour()
{
	return true;
}
bool Tree::LoadTexture()
{
	if( ! AntiMatter::Shell::FileExists( m_sBarkFile ) )
		return false;

	m_texBark = Texture( m_sBarkFile.c_str(), false );			
	
	return m_texBark.Initialized();
}
bool Tree::LoadBumpMap()
{
	if( ! AntiMatter::Shell::FileExists( m_sBumpFile ) )
		return false;

	m_texBump = Texture( m_sBumpFile.c_str(), false );

	return m_texBump.Initialized();
}
	
bool Tree::InitializeShaders()
{
	using AntiMatter::AppLog;
	using std::string;

	// iterate through each of the TreeShaderz
	// create a ShaderProgram for each
	// store the corresponding ShaderProgram address in the TreeShaderz data structure
	// elect a "current" shader (set its address in m_pCurrentShader)
	
	for( int n = WireFrame; n < BumpTextured; n ++ )
	{
		ShaderProgram * pNext = NULL;
		string			sVert = m_Shaderz.Vert( (eTreeShaders)n );
		string			sFrag = m_Shaderz.Frag( (eTreeShaders)n );

		if( CreateShaderProgram( sVert, sFrag, &pNext ) )
		{
			m_Shaderz.ShaderProg( (eTreeShaders)n, pNext);

			if( n == WireFrame )
				m_pCurrentShader = pNext;
		}
		else
		{
			AppLog::Ref().LogMsg("Tree::InitializeShaders() failed to initialize shader %s", sVert.c_str() );
			// ATLASSERT(0);
		}
	}

	return true;
}
void Tree::UninitializeShaders()
{
	// iterate through each of the TreeShaderz
	// delete the memory allocated for each ShaderProgram
	// smart pointer not allowed in this project :S
	// this code is replicated in the TreeShaderz::Uninitialize() function
	
	for( int n = WireFrame; n < BumpTextured; n ++ )
	{
		ShaderProgram* pNext = m_Shaderz.ShaderProg( (eTreeShaders) n );

		if( pNext )
		{
			delete pNext;
			pNext = NULL;
			m_Shaderz.ShaderProg( (eTreeShaders)n, NULL );
		}
	}
}

bool Tree::CreateShaderProgram( const std::string & sVert, const std::string & sFrag, ShaderProgram ** ppShaderProg )
{
	// Create a ShaderProgram object and initialize it

	using namespace std;
	using namespace AntiMatter;

	vector<ShaderDesc>				vDescs;
	vector<ShaderInputAttribute>	vArgs;
	
	ShaderDesc						VertShaderDesc;
	ShaderDesc						PixShaderDesc;
	
	ShaderInputAttribute			VertPosArg;
	ShaderInputAttribute			VertTextureArg;
	ShaderInputAttribute			VertNormalArg;
	ShaderInputAttribute			VertColorArg;

	VertShaderDesc.sFileName	= g_Cfg.ShadersDir() + sVert;
	PixShaderDesc.sFileName		= g_Cfg.ShadersDir() + sFrag;

	VertShaderDesc.nType		= Vertex;
	PixShaderDesc.nType			= Fragment;

	VertPosArg.sFieldName		= string("VertexPosition");
	VertPosArg.nFieldSize		= 3;	// not bytes, num of components
	VertPosArg.nFieldOffset		= 0;
	VertPosArg.nStride			= sizeof(CustomVertex);

	VertNormalArg.sFieldName	= string("VertexNormal");
	VertNormalArg.nFieldSize	= 3;
	VertNormalArg.nFieldOffset	= 3;
	VertNormalArg.nStride		= sizeof(CustomVertex);	

	VertTextureArg.sFieldName	= string("VertexTexCoord");
	VertTextureArg.nFieldSize	= 2;
	VertTextureArg.nFieldOffset	= 6;
	VertTextureArg.nStride		= sizeof(CustomVertex);

	VertColorArg.sFieldName		= string("VertexColour");
	VertColorArg.nFieldSize		= 4;
	VertColorArg.nFieldOffset	= 8;
	VertColorArg.nStride		= sizeof(CustomVertex);

	vDescs.push_back ( VertShaderDesc );
	vDescs.push_back ( PixShaderDesc );

	vArgs.push_back ( VertPosArg );	
	vArgs.push_back ( VertNormalArg );	
	vArgs.push_back ( VertTextureArg );

	*ppShaderProg = new ShaderProgram( m_Cylinder.Vertices(), m_Cylinder.Indices(), m_Cylinder.VertCount(), m_Cylinder.IndexCount(), vDescs, vArgs );
	if( ! (*ppShaderProg)->Initialized() )
	{
		delete *ppShaderProg;
		*ppShaderProg = NULL;
		return false;
	}

	return true;
}
void Tree::SetShaderArgs()
{
	using namespace glm;
	using namespace std;
	using namespace AntiMatter;

	if( ! m_pCurrentShader )
		return;

	// I only put these into variables for debug purposes
	GLuint nShaderProgId	= m_pCurrentShader->ShaderProgId();
	GLuint nVaoId			= m_pCurrentShader->VaoId();
	GLuint nVboId			= m_pCurrentShader->VboId();
	GLuint nVboIndexId		= m_pCurrentShader->VboIndexId();
	GLuint nTexBarkId		= m_texBark.TextureId();
	GLuint nTexBumpId		= m_texBump.TextureId();

	// Indicate which ShaderProgram to use	
	glUseProgram( nShaderProgId );

	// bind buffers
	glBindVertexArray( nVaoId );							// vertex array object
	glBindBuffer( GL_ARRAY_BUFFER,			nVboId );		// vertices
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	nVboIndexId );	// indices	


	// Assign uniform variables
	// lights
	vector<Light*> lights = this->Graph()->Lights().Lights();
	for( unsigned int x = 0; x < lights.size(); x++ )
	{
		vec4 lightPos	= lights[x]->Pos();
		vec3 lightInt	= lights[x]->Intensity();
		
		vec4 eyeLightPos = this->Graph()->Cam().V() * this->GetNodeData().W() * lightPos;

		stringstream ssP, ssI;
			
		ssP  << "lights[" << x << "].Position";
		ssI << "lights[" << x << "].Intensity";		

		string sPosition	= ssP.str();
		string sIntensity	= ssI.str();		

		m_pCurrentShader->AssignUniformVec4( sPosition,	 eyeLightPos );
		AppLog::Ref().OutputGlErrors();	
		m_pCurrentShader->AssignUniformVec3( sIntensity, lightInt );
		AppLog::Ref().OutputGlErrors();	
	}	

	// textures				
	m_pCurrentShader->AssignUniformSampler2D( string("tex"), nTexBarkId );
	AppLog::Ref().OutputGlErrors();	
	m_pCurrentShader->AssignUniformSampler2D( string("texBump"), nTexBumpId );
	AppLog::Ref().OutputGlErrors();	

	// Material
	m_pCurrentShader->AssignUniformVec3(  string("Ka"),			m_material.Ka() );
	m_pCurrentShader->AssignUniformVec3(  string("Kd"),			m_material.Kd() );
	m_pCurrentShader->AssignUniformVec3(  string("Ks"),			m_material.Ks() );
	m_pCurrentShader->AssignUniformFloat( string("rShininess"),	m_material.Shininess() );
	AppLog::Ref().OutputGlErrors();		
	
	/*
	// matrices
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_pParent->GetNodeData().W() * m_Data.W();
	glm::mat4 mModelView	= mView * mModel;

	glm::mat3 mNormal(mModelView);
	mNormal = glm::transpose(mNormal._inverse());

	m_pCurrentShader->AssignUniformMat4( string("mModelView"),	mModelView );
	m_pCurrentShader->AssignUniformMat3( string("mNormal"),		mNormal );
	m_pCurrentShader->AssignUniformMat4( string("mMVP"),		m_Data.MVP() );	
		
	AppLog::Ref().OutputGlErrors();
	*/
}
	
HRESULT Tree::Update( const float & rSecsDelta )
{
	m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();
	
	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() *
						m_Data.Stack();
	
	// Updates child nodes
	IGraphNode::UpdateChildren( rSecsDelta );
	
	return S_OK;
}
HRESULT Tree::PreRender()
{
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
	using AntiMatter::AppLog;
	
	for( LTree::iterator n = m_LTree.begin(); n != m_LTree.end(); n ++ )
	{
		Generation i = (*n);
	
		for( Generation::iterator j = i.begin(); j != i.end(); j ++ )
		{
			// get the next seg
			Segment * pNextSeg = *j;

			if( pNextSeg->Generation() >= 2 )
				break;

			// compute the scale for this segment
			float	rGenScaler;
			float	rSegScaler;
			int		nGeneration	= pNextSeg->Generation();
			mat4	mW			= mat4(1.0);
			vec3	vPosition;

			switch( nGeneration )
			{
				case 0:  rGenScaler = 1.0f;  break;
				case 1:  rGenScaler = 0.65f; break;
				case 2:  rGenScaler = 0.55f;  break;
				default: rGenScaler = 0.40f; break;
			}
			
			rSegScaler = ( m_rInitialSegLength / m_Cylinder.Length() ) * rGenScaler;
			pNextSeg->Length( rSegScaler * m_rInitialSegLength );
			

			// compute the position for this segment
			vec3 vPos;
			if( nGeneration <= 1 )
			{
				// Compute position from the tree's model matrix, then store it back into the segment
				mat4 mTmp	= m_pParent->GetNodeData().W() * m_Data.W();
				vPosition	= vec3( mTmp[3][0], mTmp[3][1], mTmp[3][2] );
				vPos		= pNextSeg->Position();
				vPosition	= vec3(0,1,0);
				pNextSeg->Position( vPosition );
			}
			else
			{
				// read parents position, then translate to the end of the parent
				// scale the direction of the parent branch by the length of the parent
				vPosition	= glm::normalize( pNextSeg->Parent()->Position() ) * (pNextSeg->Parent()->Length());
				vPos		= pNextSeg->Position();

				pNextSeg->Position( vPos );
				mW	= glm::translate( mW, vPos );
			}
			
			mW *= pNextSeg->W();			
			mW = glm::scale( mW, vec3(rSegScaler) );			
			

			// draw the branch
			if( m_pCurrentShader )
			{
				SetShaderArgs();
				
				// matrices
				mat4 mModelView = m_pGraph->Cam().V() * m_Data.Stack() * mW;
				mat4 mMVP		= m_pGraph->Proj().P() * m_pGraph->Cam().V() * m_Data.Stack() * mW;

				mat3 mNormal(mModelView);
				mNormal = glm::transpose(mNormal._inverse());

				m_pCurrentShader->AssignUniformMat4( string("mModelView"),	mModelView );
				m_pCurrentShader->AssignUniformMat3( string("mNormal"),		mNormal );
				m_pCurrentShader->AssignUniformMat4( string("mMVP"),		mMVP );

				glPolygonMode( GL_FRONT_AND_BACK, ( m_CurrentShader == WireFrame ) ? GL_LINE : GL_FILL );
				
				glDrawElements( GL_TRIANGLE_STRIP, m_Cylinder.TriCount(), GL_UNSIGNED_INT, ((char*)NULL +  0) );

				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
				AppLog::Ref().OutputGlErrors();

				glBindTexture( GL_TEXTURE_2D,			0 );
				glBindBuffer( GL_ARRAY_BUFFER,			0 );
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
				glBindVertexArray(0);
				glUseProgram(0);
			}
		}
	}	

	return S_OK;
}
HRESULT Tree::PostRender()
{
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