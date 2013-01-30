
#include "stdafx.h"
#include "House.h"
#include "IGraphNode.h"

#include "Quad.h"
#include "Tri.h"

#include "SceneGraph.h"
#include "VertexClass.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "EffectMgr.h"
#include "Effect.h"

#include <iomanip>
#include <fstream>
#include "slowglobe-cfg.h"

#include <AntiMatter\Exceptions.h>
#include <AntiMatter\ShellUtils.h>
#include <AntiMatter\AppLog.h>

#include <glm\glm.hpp>

#include <fstream>
#include <iostream>

House::House() : 
	IGraphNode			( NULL, NULL, std::string("house") ),
	m_bInitialized		( false ),
	_pQuadEffect		( NULL ),
	_pTriEffect			( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 )
{
	glex::Load();
}
House::House( SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, const std::string & sConfigFile ) :
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	_pQuadEffect		( NULL ),
	_pTriEffect			( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 ),
	m_sConfigFile		( sConfigFile )
{
	glex::Load();
	m_bInitialized = Initialize();
}
House::House( const House & rhs ) : 
	IGraphNode			( rhs.m_pGraph, rhs.m_pParent, rhs.m_sId ),
	m_bInitialized		( false ),
	_pQuadEffect		( NULL ),
	_pTriEffect			( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 ),
	m_sConfigFile		( rhs.m_sConfigFile )
{
	if( rhs.Initialized() )
		m_bInitialized = Initialize();
}
House & House::operator=( const House & rhs )
{
	if( this != &rhs )
	{
		if( this->Initialized() )
			Uninitialize();

		m_pGraph			= rhs.m_pGraph;
		m_pParent			= rhs.m_pParent;
		m_sId				= rhs.m_sId;
		m_nSunSub			= 0;
		m_nSpotlightSub		= 0;
		m_sConfigFile		= rhs.m_sConfigFile;

		if( rhs.Initialized() )
			this->Initialize();
	}

	return *this;
}
House::~House()
{
	Uninitialize();
}

bool House::Initialize()
{
	if( m_bInitialized )
		return true;

	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using namespace std;
	
	// these objects are geometry used to draw each of the required primitives.
	m_Quad	= Quad( 2.0f, 1.2f );
	m_Tri	= Tri( 1.2f, 0.8f );

	if( ! InitializeGeometry() )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s initialize geometry failed for cylinder", __FUNCTION__ );
		return false;
	}	

	if( ! GetShader(&_pQuadEffect) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize quad shader", __FUNCTION__);
		return false;
	}

	if( ! GetShader(&_pTriEffect) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize tri shader", __FUNCTION__);
		return false;
	}

	if( ! InitializeVbo(&_pQuadVbo, m_Quad) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize quad vbo", __FUNCTION__);
		return false;
	}

	if( ! InitializeVbo(&_pTriVbo, m_Tri) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize tri vbo", __FUNCTION__);
		return false;
	}

	if( ! InitializeVao( _pQuadEffect, _nQuadVao, _pQuadVbo ) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize quad vao", __FUNCTION__);
		return false;		
	}

	if( ! InitializeVao( _pTriEffect, _nTriVao, _pTriVbo ) )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize tri vao", __FUNCTION__);
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
			m_bInitialized = LoadAssets();
		}
		else
		{
			AppLog::Ref().LogMsg("%s config file invalid", __FUNCTION__ );
			Uninitialize();
			return false;
		}		
	}

	if( ! m_bInitialized )
	{
		AppLog::Ref().LogMsg("%s load assets failed", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	// load each matrix with the identity, then compute the affine transforms for each house part
	m_mW.resize(NumHouseParts);
	for( int n = 0; n < NumHouseParts; n ++ )
		m_mW[n] = glm::mat4(1.0);

	TransformGeometry();

	return m_bInitialized;
}
void House::Uninitialize()
{
	if( ! m_bInitialized )
		return;

	m_bInitialized	= false;	

	// no longer need the effect ptrs	
	_pQuadEffect	= NULL;
	_pTriEffect		= NULL;	

	// delete vertex arrays
	if( _nQuadVao )
		glDeleteVertexArrays( 1, &_nQuadVao );

	if( _nTriVao )
		glDeleteVertexArrays( 1, &_nTriVao );

	_nQuadVao = _nTriVao = 0;

	// delete textures
	for( unsigned int n = 0; n < m_Textures.size(); n ++ )
	{
		if( m_Textures[n] )
			delete m_Textures[n];
	}
	m_Textures.clear();

	// remaining assets
	m_sAssetFiles.clear();
	m_sConfigFile	= "";

	ZeroMemory( &m_matWall, sizeof(Material) );
	ZeroMemory( &m_matRoof, sizeof(Material) );		
}

void House::TransformGeometry()
{
	// moves the houseparts into position relative to one another
	// the housepart dimensions are scaled in the xz dimension
	// these calculations are only done once

	using glm::vec3;
	using glm::mat4;
	using glm::translate;
	using glm::rotate;
	using glm::scale;

	float	rHouseWidth		= 200.0f;	// front wall width
	float	rHouseHeight	= 120.0f;	// wall height
	float	rHouseDepth		= 120.0f;	// 
	mat4	mW (1.0);
			
	// front wall (translate, rotate, scale, draw)	
	mW = glm::translate	( mW, vec3(0, 0, rHouseDepth/2.0f) );
	mW = glm::rotate	( mW, 90.0f, vec3(1.0, 0.0, 0.0) );
	mW = glm::scale		( mW, vec3(100.0f, 1.0f, 100.0f ) );	
	m_mW[FrontWall] = mW;

	// right wall (translate, rotate, scale, draw)
	mW = mat4(1.0);
	mW = glm::translate	( mW, vec3(rHouseWidth/2.0f, 0, 0) );		
	mW = glm::rotate	( mW, 90.0f,	vec3(0.0f, 0.0f, 1.0f) );
	mW = glm::rotate	( mW, 180.0f,	vec3(1.0f, 0.0f, 0.0f) );
	mW = glm::rotate	( mW, 90.0f,	vec3(0.0f, 1.0f, 0.0f) );	
	mW = glm::scale		( mW, vec3( 60.0f, 1.0f, 100.0f ) );
	m_mW[RightWall] = mW;

	// back wall (translate, rotate, scale, draw)
	mW = mat4(1.0);
	mW = glm::translate	( mW, vec3(0, 0, -rHouseDepth/2.0f) );
	mW = glm::rotate	( mW, -90.0f,	vec3(1.0f, 0.0f, 0.0f) );
	mW = glm::rotate	( mW, 180.0f,	vec3(0.0f, 1.0f, 0.0f) );	
	mW = glm::scale		( mW, vec3(100.0f, 1.0f, 100.0f ) );	
	m_mW[BackWall] = mW;

	// left wall (translate, rotate, scale, draw)
	mW = mat4(1.0);
	mW = glm::translate	( mW, vec3(-rHouseWidth/2.0f, 0, 0) );	
	mW = glm::rotate	( mW, -90.0f,	vec3(0.0f, 0.0f, 1.0f) );
	mW = glm::rotate	( mW, 180.0f,	vec3(1.0f, 0.0f, 0.0f) );
	mW = glm::rotate	( mW, 90.0f,	vec3(0.0f, 1.0f, 0.0f) );
	mW = glm::scale		( mW, vec3( 60.0f, 1.0f, 100.0f ) );
	m_mW[LeftWall] = mW;

	// front roof slope (translate, rotate, scale, draw)
	mW = mat4(1.0);
	mW = glm::translate	( mW, vec3(0, (rHouseHeight/2.0f)+30.0f, 0) );
	mW = glm::translate	( mW, vec3(0, 0, rHouseDepth/4.0f) );
	mW = glm::rotate	( mW, 45.0f, vec3(1.0, 0.0, 0.0) );
	mW = glm::scale		( mW, vec3(100.0f, 1.0f, 75.0f) );
	m_mW[FrontRoofSlope] = mW;

	// back roof slope (translate, rotate, scale, draw)
	mW = mat4(1.0);
	mW = glm::translate	( mW, vec3(0, (rHouseHeight/2.0f)+30.0f, 0) );
	mW = glm::translate	( mW, vec3(0, 0, -rHouseDepth/4.0f) );
	mW = glm::rotate	( mW, -45.0f, vec3(1.0, 0.0, 0.0) );
	mW = glm::scale		( mW, vec3(100.0f, 1.0f, 75.0f) );
	m_mW[BackRoofSlope] = mW;

	// right roof side 1(translate, rotate, scale, draw)
	mW = mat4(1.0);
	mW = glm::translate	( mW, vec3(rHouseWidth/2.0, 0.0, 0.0) );
	mW = glm::translate	( mW, vec3(0.0, rHouseHeight, 0.0) );
	mW = glm::translate	( mW, vec3(0.0f, -60.0f, 2) );

	mW = glm::rotate	( mW, 90.0f,	vec3(0.0, 0.0, 1.0) );
	mW = glm::rotate	( mW, 180.0f,	vec3(1.0, 0.0, 0.0) );
	mW = glm::rotate	( mW, 225.0f,	vec3(0.0, 1.0, 0.0) );
	mW = glm::scale		( mW, vec3(40.0f, 1.0f, 70.0f) );
	m_mW[RightRoofSide] = mW;


	// left roof side (translate, rotate, scale, draw)
	mW = mat4(1.0);
	mW = glm::translate	( mW, vec3(-rHouseWidth/2.0, 0.0, 0.0) );
	mW = glm::translate	( mW, vec3(0.0, rHouseHeight, 0.0) );
	mW = glm::translate	( mW, vec3(0.0f, -60.0f, -2) );

	mW = glm::rotate	( mW, 90.0f,	vec3(0.0, 0.0, 1.0) );
	mW = glm::rotate	( mW, -90.0f,	vec3(0.0, 1.0, 0.0) );
	mW = glm::rotate	( mW, -45.0f,	vec3(0.0, 1.0, 0.0) );
	mW = glm::scale		( mW, vec3(40.0f, 1.0f, 70.0f) );
	m_mW[LeftRoofSide] = mW;
}

// Shader
bool House::InitializeGeometry()
{	
	m_Quad	= Quad( 2.0f, 1.2f );
	m_Tri	= Tri( 1.2f, 0.8f );
	return true;
}
bool House::InitializeVbo( Vbo<CustomVertex> **ppVbo, IGeometry & geometry )
{
	using AntiMatter::AppLog;

	(*ppVbo) = new Vbo<CustomVertex> ( 
		geometry.VertCount(), 
		geometry.Vertices(), 
		geometry.IndexCount(), 
		geometry.Indices() 
	);

	if( ! (*ppVbo)->Initialized() )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize vertex buffer for object geometry", __FUNCTION__ );
		return false;
	}

	return true;
}
bool House::InitializeVao( Effect* pEffect, GLuint & nVaoId, Vbo<CustomVertex>* pVbo )
{
	using AntiMatter::AppLog;

	glUseProgram( pEffect->Id() );

	glGenVertexArrays( 1, &nVaoId );
	glBindVertexArray( nVaoId );	

	glBindBuffer( GL_ARRAY_BUFFER,			pVbo->Id() );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	pVbo->IndexId() );
	
	// set the VBO attribute pointers
	GLuint i = 0;
	
	for( auto n = pEffect->Attributes().begin(); n != pEffect->Attributes().end(); ++ n )
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

bool House::GetShader( Effect** ppEffect )
{
	using std::string;

	bool bRet = false;

	// warning C4482: nonstandard extension used: enum 'Effect::EffectBuildState' used in qualified name
#pragma warning (push)
#pragma warning (disable: 4482)
	if( EffectMgr::Ref().Find( string("textured-phong"), ppEffect ) )
	{
		if( (*ppEffect)->BuildState() == Effect::EffectBuildState::Linked )
		{
			m_nSunSub		= glGetSubroutineIndex( (*ppEffect)->Id(), GL_FRAGMENT_SHADER, "Sunlight" );
			m_nSpotlightSub = glGetSubroutineIndex( (*ppEffect)->Id(), GL_FRAGMENT_SHADER, "Spotlights" );
			bRet			= true;
		}
	}
#pragma warning (pop)

	return bRet;
}
bool House::SetShaderArgs( Effect* pEffect, const GLuint nVaoId, const Vbo<CustomVertex>* pVbo, const Texture* pTex, const Texture* pBump, const Material & mtl )
{
	using namespace glm;
	using namespace std;
	using namespace AntiMatter;

	if( ! pEffect )
		return false;
	
	// Select Shader
	glUseProgram( pEffect->Id() );	

	// bind buffers
	glBindVertexArray( nVaoId );
	glBindBuffer( GL_ARRAY_BUFFER,			pVbo->Id() );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	pVbo->IndexId() );
	
	// Assign uniform variables

	// camera position (transformed to view space)
	vec3 vCamPos = vec3(Graph()->Cam().V() * vec4( Graph()->Cam().Pos(), 1.0));
	pEffect->AssignUniformVec3( string("vViewPosition"), vCamPos );

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

		pEffect->AssignUniformVec4( sPosition, vLightPos );
		pEffect->AssignUniformVec3( sLa, la );
		pEffect->AssignUniformVec3( sLd, ld );
		pEffect->AssignUniformVec3( sLs, ls );
		pEffect->AssignUniformVec3( sDir, vDir );
		pEffect->AssignUniformFloat( sExp, rExp );
		pEffect->AssignUniformFloat( sCut, rCutOff );
	}

	// Select light subroutine
	if( m_pGraph->Lights().GetLightsState() == SceneLights::LightsState::sun )
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSunSub );
	else
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSpotlightSub );

	// Material
	pEffect->AssignUniformVec3(  string("material.Ka"),			mtl.Ka() );
	pEffect->AssignUniformVec3(  string("material.Kd"),			mtl.Kd() );
	pEffect->AssignUniformVec3(  string("material.Ks"),			mtl.Ks() );
	pEffect->AssignUniformFloat( string("material.rShininess"),	mtl.Shininess() );

	// textures	
	pEffect->AssignUniformSampler2D( string("tex"),		pTex->TextureId() );
	pEffect->AssignUniformSampler2D( string("texBump"),	pBump->TextureId() );	

	// matrices (these args are set per-primitive in the DrawPrimitive calls)
	/*
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mView * mModel;

	glm::mat3 mNormal(mModelView);
	mNormal = glm::transpose(mNormal._inverse());

	pEffect->AssignUniformMat4( string("mModelView"),	mModelView );
	pEffect->AssignUniformMat3( string("mNormal"),		mNormal );
	pEffect->AssignUniformMat4( string("mMVP"),			m_Data.MVP() );
	*/

	AppLog::Ref().OutputGlErrors();

	return true;
}

// IGraphNode
HRESULT House::Update( const float & rSecsDelta )
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
HRESULT House::PreRender()
{
	return S_OK;
}
HRESULT House::Render()
{
	// enum HouseParts		{ FrontWall, RightWall, BackWall, LeftWall, FrontRoofSlope, BackRoofSlope, RightRoofSide, LeftRoofSide, NumHouseParts };
	// I didn't use loops here to make the DrawPrimitive calls for 2 reasons
	// 1. debugging - it is much easier to comment out individual lines and teak things
	// 2. Draw is called frequently.  The fewer loop calculations the better

	// draw the walls
	SetShaderArgs( _pQuadEffect, _nQuadVao, _pQuadVbo, m_Textures[WallTexture], m_Textures[WallBump], m_matWall );
	DrawPrimitive( _pQuadEffect, &m_Quad, m_mW[FrontWall] );
	DrawPrimitive( _pQuadEffect, &m_Quad, m_mW[RightWall] );
	DrawPrimitive( _pQuadEffect, &m_Quad, m_mW[BackWall] );
	DrawPrimitive( _pQuadEffect, &m_Quad, m_mW[LeftWall] );

	// Release buffers set in SetShaderArgs()
	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
	glBindVertexArray(0);
	glUseProgram(0);

	// draw the roof
	SetShaderArgs( _pQuadEffect, _nQuadVao, _pQuadVbo, m_Textures[RoofTexture], m_Textures[RoofBump], m_matRoof );
	
	// slopes
	DrawPrimitive( _pQuadEffect, &m_Quad, m_mW[FrontRoofSlope] );
	DrawPrimitive( _pQuadEffect, &m_Quad, m_mW[BackRoofSlope] );

	// sides
	// for some reason, I always got right angled triangles with these things.
	// not sure why.  will come back to after bump mapping
	DrawPrimitive( _pTriEffect, &m_Tri, m_mW[RightRoofSide] );
	DrawPrimitive( _pTriEffect, &m_Tri, m_mW[LeftRoofSide] );

	return S_OK;
}
HRESULT House::PostRender()
{
	// Release buffers set in SetShaderArgs()
	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
	glBindVertexArray(0);
	glUseProgram(0);

	return S_OK;
}
HRESULT House::DrawItem()
{
	// Execute default behaviour, which renders all child nodes first
	IGraphNode::DrawItem();

	PreRender();
	Render();
	PostRender();

	return S_OK;
}
	
bool House::DrawPrimitive( Effect* pEffect, IGeometry* pShape, const glm::mat4 & mW )
{
	using AntiMatter::AppLog;
	using std::string;
	using glm::mat4;
	using glm::mat3;
	using glm::vec3;

	// matrices mModelView, mMVP, mNormal	
	mat4 mModelView	= m_pGraph->Cam().V() * m_Data.Stack() * mW;
	mat4 mMVP		= m_pGraph->Proj().P() * mModelView;

	glm::mat3 mNormal(mModelView);
	mNormal = glm::transpose(mNormal._inverse());

	pEffect->AssignUniformMat4( string("mModelView"),	mModelView );
	pEffect->AssignUniformMat3( string("mNormal"),		mNormal );
	pEffect->AssignUniformMat4( string("mMVP"),			mMVP );

	// execute the shader program
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements( GL_TRIANGLES, pShape->IndexCount(), GL_UNSIGNED_SHORT, ((char*)NULL) );

	AppLog::Ref().OutputGlErrors();	
	
	return true;
}

bool House::LoadAssets()
{
	// LoadAssets() assumes that the asset vector has already been initialized 
	// via the streaming operator
	using AntiMatter::Shell::FileExists;
	using AntiMatter::AppLog;
	using std::string;
	using std::vector;
	typedef vector<string> Assets;

	for( int n = 0; n < NumAssets; n ++ )
	{
		string sTexture = g_Cfg.AssetsDir()  + m_sAssetFiles[n];

		if( FileExists(sTexture) )
		{
			Texture* pTexture = new (std::nothrow) Texture(sTexture);
			if( ! pTexture )
			{
				AppLog::Ref().LogMsg( "%s failed to load texture %s", __FUNCTION__, sTexture.c_str() );
				return false;
			}
			m_Textures.push_back( pTexture );
		}
	}

	return true;
}
void House::AssignAssets(	const std::vector<std::string> & sAssets, 
							const Material & matWalls, 
							const Material & matRoof )
{	
	// AssignAssets() is invoked by stream in operator >>
	// to populate the assets list
	// This _only_ populates the assets list.  Loading of assets is specifically called for in Initialize();

	// copy assets strings
	m_sAssetFiles.clear();
	m_sAssetFiles.resize( sAssets.size() );
	std::copy( sAssets.begin(), sAssets.end(), m_sAssetFiles.begin() );	

	m_matWall = matWalls;
	m_matRoof = matRoof;
}

std::ostream & operator << ( std::ostream & out, const House & r )
{
	using AntiMatter::AppLog;
	using std::string;
	using std::vector;

	//char sBuffer[1024];

	try
	{
		out << "house" << " ";

		for( int n = 0; n < NumAssets; n ++ )	
			out << r.AssetFiles()[n].c_str() << std::endl;

		out << r.RoofMtl();
		out << r.WallMtl();

		out << std::endl;
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated (probably disk) exception", __FUNCTION__ );
	}

	return out;
}
std::istream & operator >> ( std::istream & in, House & r )
{
	using AntiMatter::AppLog;
	using std::string;
	using std::vector;

	char sBuffer[1024];

	try
	{
		in >> sBuffer; 

		std::vector<std::string> sAssets;
		Material matWalls;
		Material matRoof;

		if( strcmp(sBuffer, "house") == 0 )
		{
			sAssets.reserve(NumAssets);
		
			for( int n = 0; n < NumAssets; n ++ )
			{
				in >> sBuffer; 
				sAssets.push_back( string(sBuffer) );
			}		

			in >> matWalls;
			in >> matRoof;

			r.AssignAssets( sAssets, matWalls, matRoof );
		}
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated (probably disk) exception", __FUNCTION__ );
	}
		
	return in;
}