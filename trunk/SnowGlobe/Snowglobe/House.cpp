
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
#include "ShaderProgram.h"

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
	m_pQuadShader		( NULL ),
	m_pTriShader		( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 )
{
}
House::House( SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, const std::string & sConfigFile ) :
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	m_pQuadShader		( NULL ),
	m_pTriShader		( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 ),
	m_sConfigFile		( sConfigFile )
{
	m_bInitialized = Initialize();
}
House::House( const House & rhs ) : 
	IGraphNode			( rhs.m_pGraph, rhs.m_pParent, rhs.m_sId ),
	m_bInitialized		( false ),
	m_pQuadShader		( NULL ),
	m_pTriShader		( NULL ),
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

	glex::Load();

	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using namespace std;
	
	// these objects are geometry used to draw each of the required primitives.
	m_Quad	= Quad( 2.0f, 1.2f );
	m_Tri	= Tri( 1.2f, 0.8f );

	bool bSuccess = false;

	bSuccess = CreateShader( &m_pQuadShader, m_Quad );
	if (! bSuccess )
	{
		AppLog::Ref().LogMsg("%s failed to load shader for quad geometry", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	bSuccess = CreateShader( &m_pTriShader, m_Tri );
	if( ! bSuccess )
	{
		AppLog::Ref().LogMsg("%s failed to load shader for quad geometry", __FUNCTION__ );
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
	m_bInitialized	= false;

	m_sAssetFiles.clear();
	m_sConfigFile	= "";

	ZeroMemory(&m_matWall, sizeof(Material));
	ZeroMemory(&m_matRoof, sizeof(Material));	

	// delete textures
	for( unsigned int n = 0; n < m_Textures.size(); n ++ )
	{
		if( m_Textures[n] )
			delete m_Textures[n];
	}
	m_Textures.clear();

	// delete shaders
	if( m_pQuadShader )
	{
		delete m_pQuadShader;
		m_pQuadShader = NULL;
	}
	if( m_pTriShader )
	{
		delete m_pTriShader;
		m_pTriShader = NULL;
	}		
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
bool House::CreateShader( ShaderProgram** ppShader, IGeometry & pShape )
{
	if( ! ppShader )
		return false;
	
	using namespace std;
	using namespace AntiMatter;
	
	vector<ShaderDesc>				vDescs;
	vector<ShaderInputAttribute>	vArgs;

	ShaderDesc						VertShaderDesc;
	ShaderDesc						PixShaderDesc;

	ShaderInputAttribute			VertPosArg;	
	ShaderInputAttribute			VertNormalArg;
	ShaderInputAttribute			VertTextureArg;


	VertShaderDesc.sFileName	= g_Cfg.ShadersDir() + string("textured-phong.vert");
	PixShaderDesc.sFileName		= g_Cfg.ShadersDir() + string("textured-phong.frag");

	VertShaderDesc.nType		= Vertex;
	PixShaderDesc.nType			= Fragment;

	VertPosArg.sFieldName		= string("VertexPosition");
	VertPosArg.nFieldSize		= 3;	// not bytes, num of components	VertPosArg.nFieldOffset		= 0;
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

	vDescs.push_back ( VertShaderDesc );
	vDescs.push_back ( PixShaderDesc );

	vArgs.push_back ( VertPosArg );	
	vArgs.push_back ( VertNormalArg );	
	vArgs.push_back ( VertTextureArg );

	*ppShader = new ShaderProgram( 
		pShape.Vertices(), 
		pShape.Indices(), 
		pShape.VertCount(), 
		pShape.IndexCount(), 
		vDescs, 
		vArgs 
	);

	if( (*ppShader)->Initialized() )
	{
		m_nSunSub		= glGetSubroutineIndex( (*ppShader)->ShaderProgId(), GL_FRAGMENT_SHADER, "Sunlight" );
		m_nSpotlightSub = glGetSubroutineIndex( (*ppShader)->ShaderProgId(), GL_FRAGMENT_SHADER, "Spotlights" );
	}
	else
	{
		delete *ppShader;
		*ppShader = NULL;
		return false;
	}

	return true;
}
void House::SetShaderArgs( ShaderProgram * pShader, Texture* pTex, Texture* pBump, const Material & mat )
{
	using namespace glm;
	using namespace AntiMatter;
	using namespace std;

	// I only put these into variables for debug purposes
	GLuint nShaderProgId	= pShader->ShaderProgId();
	GLuint nVaoId			= pShader->VaoId();
	GLuint nVboId			= pShader->VboId();
	GLuint nVboIndexId		= pShader->VboIndexId();
	GLuint nTexId			= pTex->TextureId();
	GLuint nBumpId			= pBump->TextureId();

	// Indicate which ShaderProgram to use	
	glUseProgram( nShaderProgId );
	if( m_pGraph->Lights().GetLightsState() == SceneLights::LightsState::sun )
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSunSub );
	else
		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSpotlightSub );

	// bind buffers
	glBindVertexArray( nVaoId );							// vertex array object
	glBindBuffer( GL_ARRAY_BUFFER,			nVboId );		// vertices
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	nVboIndexId );	// indices	

		
	// -- Assign uniform variables ------

	// vViewPosition (camera position transformed into view space)
	vec3 vCamPos = vec3(Graph()->Cam().V() * vec4( Graph()->Cam().Pos(), 1.0));
	pShader->AssignUniformVec3( string("vViewPosition"), vCamPos );


	// lights - light[0-3] = spotlights, light[4] = sun
	vector<Light*> lights = this->Graph()->Lights().Lights();
	for( unsigned int x = 0; x < lights.size(); x++ )
	{
		glm::vec4 vLightPos	= Graph()->Cam().V() * lights[x]->Pos();
		glm::vec3 la		= lights[x]->La();
		glm::vec3 ld		= lights[x]->Ld();
		glm::vec3 ls		= lights[x]->Ls();		

		stringstream ssP, ssLa, ssLd, ssLs;			
			
		ssP  << "lights[" << x << "].Position";
		ssLa << "lights[" << x << "].La";
		ssLd << "lights[" << x << "].Ld";
		ssLs << "lights[" << x << "].Ls";

		string sPosition	= ssP.str();
		string sLa			= ssLa.str();
		string sLd			= ssLd.str();
		string sLs			= ssLs.str();

		pShader->AssignUniformVec4( sPosition,	 vLightPos );
		pShader->AssignUniformVec3( sLa, la );
		pShader->AssignUniformVec3( sLd, ld );
		pShader->AssignUniformVec3( sLs, ls );
	}
	AppLog::Ref().OutputGlErrors();


	// Material
	pShader->AssignUniformVec3(  string("material.Ka"),			mat.Ka() );
	pShader->AssignUniformVec3(  string("material.Kd"),			mat.Kd() );
	pShader->AssignUniformVec3(  string("material.Ks"),			mat.Ks() );
	pShader->AssignUniformFloat( string("material.rShininess"),	mat.Shininess() );		
	AppLog::Ref().OutputGlErrors();	

	
	// textures	
	pShader->AssignUniformSampler2D( string("tex"),		nTexId );		
	pShader->AssignUniformSampler2D( string("texBump"),	nBumpId );
	AppLog::Ref().OutputGlErrors();	


	// matrices
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mView * mModel;

	glm::mat3 mNormal(mModelView);
	mNormal = glm::transpose(mNormal._inverse());

	pShader->AssignUniformMat4( string("mModelView"),	mModelView );
	pShader->AssignUniformMat3( string("mNormal"),		mNormal );
	pShader->AssignUniformMat4( string("mMVP"),			m_Data.MVP() );
	AppLog::Ref().OutputGlErrors();
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
	SetShaderArgs( m_pQuadShader, m_Textures[WallTexture], m_Textures[WallBump], m_matWall );
	DrawPrimitive( m_pQuadShader, &m_Quad, m_mW[FrontWall], m_Textures[WallTexture], m_Textures[WallBump] );
	DrawPrimitive( m_pQuadShader, &m_Quad, m_mW[RightWall], m_Textures[WallTexture], m_Textures[WallBump] );
	DrawPrimitive( m_pQuadShader, &m_Quad, m_mW[BackWall],  m_Textures[WallTexture], m_Textures[WallBump] );
	DrawPrimitive( m_pQuadShader, &m_Quad, m_mW[LeftWall],  m_Textures[WallTexture], m_Textures[WallBump] );

	// Release buffers set in SetShaderArgs()
	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
	glBindVertexArray(0);
	glUseProgram(0);	

	// draw the roof
	SetShaderArgs( m_pQuadShader, m_Textures[RoofTexture], m_Textures[RoofBump], m_matRoof );
	
	// slopes
	DrawPrimitive( m_pQuadShader, &m_Quad, m_mW[FrontRoofSlope], m_Textures[RoofTexture], m_Textures[RoofBump] );
	DrawPrimitive( m_pQuadShader, &m_Quad, m_mW[BackRoofSlope], m_Textures[RoofTexture], m_Textures[RoofBump] );

	// sides
	// for some reason, I always got right angled triangles with these things.
	// not sure why.  will come back to after bump mapping
	DrawPrimitive( m_pTriShader, &m_Tri, m_mW[RightRoofSide], m_Textures[RoofTexture], m_Textures[RoofBump] );
	DrawPrimitive( m_pTriShader, &m_Tri, m_mW[LeftRoofSide], m_Textures[RoofTexture], m_Textures[RoofBump] );

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
	
HRESULT House::DrawPrimitive( ShaderProgram* pShader, IGeometry* pShape, glm::mat4 & mW, Texture* pTexture, Texture* pBump )
{
	UNREFERENCED_PARAMETER(pBump);
	UNREFERENCED_PARAMETER(pTexture);

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

	pShader->AssignUniformMat4( string("mModelView"),	mModelView );
	pShader->AssignUniformMat3( string("mNormal"),		mNormal );
	pShader->AssignUniformMat4( string("mMVP"),			mMVP );

	// execute the shader program
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements( GL_TRIANGLES, pShape->IndexCount(), GL_UNSIGNED_SHORT, ((char*)NULL) );

	AppLog::Ref().OutputGlErrors();	
	
	return S_OK;
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