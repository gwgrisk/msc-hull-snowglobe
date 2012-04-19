
#include "stdafx.h"
#include "Terrain.h"
#include "Plane.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "ShaderProgram.h"

#include "VertexClass.h"
#include "SceneGraph.h"
#include "IGraphNode.h"

#include <GL\GL.h>
#include <AntiMatter\ShellUtils.h>
#include <AntiMatter\AppLog.h>
#include <math.h>
#include <vector>
#include <string>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "slowglobe-cfg.h"

#include <fstream>
#include "InputMgr.h"


Terrain::Terrain() :
	IGraphNode			( NULL, NULL, std::string("terrain") ),
	m_bInitialized		( false ),
	m_nTextureId		( 0 ),
	m_nVertCols			( 0 ),
	m_nVertRows			( 0 ),	
	m_rCellWidth		( 0.0f ),
	m_rCellHeight		( 0.0f ),
	m_rCellDepth		( 0.0f ),
	m_pShaderProgram	( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 ),
	m_bOldShader		( false )
{	
}
	
Terrain::Terrain(	
	SceneGraph*			pGraph, 
	IGraphNode*			pParent, 
	const std::string & sId, 
	int					nVertCols, 
	int					nVertRows, 
	const float			rCellWidth,
	const float			rCellHeight,
	const float			rCellDepth
) : 
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	m_nTextureId		( 0 ),
	m_nVertCols			( nVertCols ),
	m_nVertRows			( nVertRows ),
	m_rCellWidth		( rCellWidth ),
	m_rCellHeight		( rCellHeight ),
	m_rCellDepth		( rCellDepth ),
	m_pShaderProgram	( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 ),
	m_bOldShader		( false )
{
	Initialize();
}

Terrain::Terrain(	
	SceneGraph *		pGraph, 
	IGraphNode *		pParent, 
	const std::string & sId, 
	const std::string & sHeightField, 
	const std::string & sTexture,
	const std::string & sAlphaMap,
	const int			nVertCols, 
	const int			nVertRows, 
	const float			rCellWidth, 
	const float			rCellHeight, 
	const float			rCellDepth 
) : 
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	m_sHeightField		( sHeightField ),
	m_sTexture			( sTexture ),	
	m_sAlphaMap			( sAlphaMap ),
	m_nTextureId		( 0 ),
	m_nVertCols			( nVertCols ),
	m_nVertRows			( nVertRows ),
	m_rCellWidth		( rCellWidth ),
	m_rCellHeight		( rCellHeight ),
	m_rCellDepth		( rCellDepth ),
	m_pShaderProgram	( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 ),
	m_bOldShader		( false )
{
	Initialize();
}
		
Terrain::Terrain( const Terrain & rhs ) :
	IGraphNode			( rhs.m_pGraph, rhs.m_pParent, rhs.Id() ),
	m_nTextureId		( rhs.m_nTextureId ),
	m_nVertCols			( rhs.m_nVertCols ),
	m_nVertRows			( rhs.m_nVertRows ),
	m_rCellWidth		( rhs.m_rCellWidth ),
	m_rCellHeight		( rhs.m_rCellHeight ),
	m_rCellDepth		( rhs.m_rCellDepth ),
	m_sHeightField		( rhs.m_sHeightField ),
	m_sTexture			( rhs.m_sTexture ),
	m_sAlphaMap			( rhs.m_sAlphaMap ),
	m_pShaderProgram	( NULL ),
	m_nSunSub			( 0 ),
	m_nSpotlightSub		( 0 ),
	m_bOldShader		( rhs.m_bOldShader )

{
	if( rhs.Initialized() )
		Initialize();
}
Terrain & Terrain::operator=( const Terrain & rhs )
{
	if( this != &rhs )
	{
		if( this->m_bInitialized )
			this->Uninitialize();
				
		m_pGraph			= rhs.m_pGraph;
		m_pParent			= rhs.m_pParent;
		m_sId				= rhs.m_sId;

		m_nTextureId		= rhs.m_nTextureId;
		m_nVertCols			= rhs.m_nVertCols;
		m_nVertRows			= rhs.m_nVertRows;
		m_rCellWidth		= rhs.m_rCellWidth;
		m_rCellHeight		= rhs.m_rCellHeight;
		m_rCellDepth		= rhs.m_rCellDepth;	
		m_sHeightField		= rhs.m_sHeightField;
		m_sTexture			= rhs.m_sTexture;
		m_sAlphaMap			= rhs.m_sAlphaMap;
		m_nSunSub			= rhs.m_nSunSub;
		m_nSpotlightSub		= rhs.m_nSpotlightSub;
		m_bOldShader		= rhs.m_bOldShader;

		if( rhs.Initialized() )
			this->Initialize();
	}

	return *this;
}
Terrain::~Terrain()
{
	try
	{
		Uninitialize();
	}
	catch(...)
	{
		// Parasoft pacification
	}
}

bool Terrain::Initialize()
{
	// Create an xz plane (geometric object that defines the terrain)
	// Apply texture
	// Amend plane UV co-ords in order to accomodate texture mapping requirements
	// Apply heightfield to terrain geometry

	using namespace AntiMatter;
	using namespace std;
	using glm::vec3;

	glex::Load();

	m_bOldShader = false;

	if( (m_nVertRows == 0) || (m_nVertCols == 0) || 
		(fabs(m_rCellWidth) == 0.0f) || (fabs(m_rCellHeight) == 0.0f) || (fabs(m_rCellDepth) == 0.0f) )
			return false;	

	// material
	m_material.Ka(vec3(1.0, 1.0, 1.0));
	m_material.Kd(vec3(1.0, 1.0, 1.0));
	m_material.Ks(vec3(0.1, 0.2, 0.1));
	m_material.Shininess( 1.0f );


	// geometry
	m_Plane = Plane(m_nVertCols, m_nVertRows, m_rCellWidth, m_rCellHeight, m_rCellDepth);
	if( ! m_Plane.Initialized() )
	{
		AppLog::Ref().LogMsg("Terrain::Initialize() failed: Plane::Initialize() failed.");
		return false;
	}

	ApplyTexture();
	ApplyAlphaMap();
	ApplyHeightField();
	CreateShader();

	if( m_pShaderProgram )
		m_bInitialized = m_pShaderProgram->Initialized();
	
	return m_bInitialized;
}
void Terrain::Uninitialize()
{	
	m_bInitialized = false;

	m_sHeightField.clear();
	m_sTexture.clear();	

	m_nVertCols		= 0;
	m_nVertRows		= 0;
	m_rCellWidth	= 0.0f;
	m_rCellHeight	= 0.0f;
	m_rCellDepth	= 0.0f;

	if( m_Plane.Initialized() )
		m_Plane.Uninitialize();	

	if( m_pShaderProgram )
	{
		if( m_pShaderProgram->Initialized() )
			m_pShaderProgram->Uninitialize();

		delete m_pShaderProgram;
		m_pShaderProgram = NULL;
	}	
}

void Terrain::ApplyTexture()
{
	if( AntiMatter::Shell::FileExists( m_sTexture ) )
		m_texGrass = Texture( m_sTexture.c_str(), false );

	// we want the texture to repeat, so we'll need to ensure that if the texture is smaller than the 
	// geometry, that the UV coords tile the texture and don't just stretch it to the four corners 

	// UV coords are part of the Plane object's CustomVertex array
	
}
void Terrain::ApplyHeightField()
{	
	// for each vertex..
	// set the y value of position (e.g. height value) to combination of heightfield RGB values, scaled by cell height
	using namespace AntiMatter;

	unsigned char	r,g,b;
	int				nVI;
	gxbase::Image	img;

	if( Shell::FileExists( m_sHeightField ) )
	{
		img.Load( m_sHeightField.c_str() );

		if( img.IsValid() )
		{
			for( int y = 0; y < m_nVertRows; y ++ )
			{
				for( int x = 0; x < m_nVertCols; x ++ )
				{
					img.GetPixel3b( (int)(x * m_rCellWidth), (int)(y * m_rCellHeight), r, g, b);

					nVI = (m_nVertCols * y) + x;
					float rYVal = (float)(r+g+b) * m_rCellDepth;
					(*m_Plane.Vertices())[nVI].m_Position[1] = rYVal;
				}
			}
		}

		img.Free();

		// m_Plane.RecalcNormalMap();
	}		
}
void Terrain::ApplyAlphaMap()
{
	using namespace AntiMatter;

	if( Shell::FileExists( m_sAlphaMap ) )
		m_texAlphaMap = Texture( m_sAlphaMap );
}
	
bool Terrain::CreateShader()
{
	using namespace std;
	using namespace AntiMatter;

	// Create a ShaderProgram object and initialize it for the terrain object
	// This is a candidate for a factory object / scripting
	vector<ShaderDesc>				vDescs;
	vector<ShaderInputAttribute>	vArgs;
	ShaderDesc						VertShaderDesc;
	ShaderDesc						PixShaderDesc;
	ShaderInputAttribute			VertPosArg;
	ShaderInputAttribute			VertTextureArg;
	ShaderInputAttribute			VertNormalArg;
	ShaderInputAttribute			VertColorArg;	
	
	VertShaderDesc.sFileName	= g_Cfg.ShadersDir() + string("textured-phong.vert");
	PixShaderDesc.sFileName		= g_Cfg.ShadersDir() + string("textured-phong-alphamap.frag");

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

	vDescs.push_back ( VertShaderDesc );
	vDescs.push_back ( PixShaderDesc );

	vArgs.push_back ( VertPosArg );	
	vArgs.push_back ( VertNormalArg );	
	vArgs.push_back ( VertTextureArg );

	m_pShaderProgram = new ShaderProgram( m_Plane.Vertices(), m_Plane.Indices(), m_Plane.VertCount(), m_Plane.IndexCount(), vDescs, vArgs );
	if( m_pShaderProgram->Initialized() )
	{
		m_nSunSub		= glGetSubroutineIndex( m_pShaderProgram->ShaderProgId(), GL_FRAGMENT_SHADER, "Sunlight" );
		m_nSpotlightSub = glGetSubroutineIndex( m_pShaderProgram->ShaderProgId(), GL_FRAGMENT_SHADER, "Spotlights" );
	}
	else	
	{
		delete m_pShaderProgram;
		m_pShaderProgram = NULL;
		return false;
	}

	return true;
}
void Terrain::SetShaderArgs()
{
	using namespace glm;
	using namespace AntiMatter;
	using namespace std;

	// I only put these into variables for debug purposes
	GLuint nShaderProgId	= m_pShaderProgram->ShaderProgId();
	GLuint nVaoId			= m_pShaderProgram->VaoId();
	GLuint nVboId			= m_pShaderProgram->VboId();
	GLuint nVboIndexId		= m_pShaderProgram->VboIndexId();
	GLuint nTexId			= m_texGrass.TextureId();
	GLuint nAlphaId			= m_texAlphaMap.TextureId();

	// Indicate which ShaderProgram to use	
	glUseProgram( nShaderProgId );
	glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_nSunSub );

	// bind buffers
	glBindVertexArray( nVaoId );							// vertex array object
	glBindBuffer( GL_ARRAY_BUFFER,			nVboId );		// vertices
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	nVboIndexId );	// indices		
		

	// -- Assign uniform variables ------
	
	// vViewPosition (camera position transformed into view space)
	vec3 vCamPos = vec3(Graph()->Cam().V() * m_Data.W() * vec4( Graph()->Cam().Pos(), 1.0));
	m_pShaderProgram->AssignUniformVec3( string("vViewPosition"), vCamPos );

	// lights - light[0-3] = spotlights, light[4] = sun
	vector<Light*> lights = this->Graph()->Lights().Lights();
	for( unsigned int x = 0; x < lights.size(); x++ )
	{
		glm::vec4 vLightPos	= Graph()->Cam().V() * m_Data.W() * lights[x]->Pos();
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

		m_pShaderProgram->AssignUniformVec4( sPosition,	 vLightPos );
		m_pShaderProgram->AssignUniformVec3( sLa, la );
		m_pShaderProgram->AssignUniformVec3( sLd, ld );
		m_pShaderProgram->AssignUniformVec3( sLs, ls );
	}
	AppLog::Ref().OutputGlErrors();

	// material data
	m_pShaderProgram->AssignUniformVec3(  string("material.Ka"),			m_material.Ka() );
	m_pShaderProgram->AssignUniformVec3(  string("material.Kd"),			m_material.Kd() );
	m_pShaderProgram->AssignUniformVec3(  string("material.Ks"),			m_material.Ks() );
	m_pShaderProgram->AssignUniformFloat( string("material.rShininess"),	m_material.Shininess() );
	AppLog::Ref().OutputGlErrors();

	// textures
	m_pShaderProgram->AssignUniformSampler2D( string("tex"),			nTexId );
	m_pShaderProgram->AssignUniformSampler2D( string("texAlpha"),		nAlphaId );
	AppLog::Ref().OutputGlErrors();
	
	// matrices
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mModel * mView;

	glm::mat3 mNormal(mModelView);
	mNormal = glm::transpose(mNormal._inverse());

	m_pShaderProgram->AssignUniformMat4( string("mModelView"),	mModelView );
	m_pShaderProgram->AssignUniformMat3( string("mNormal"),		mNormal );
	m_pShaderProgram->AssignUniformMat4( string("mMVP"),		m_Data.MVP() );
	AppLog::Ref().OutputGlErrors();
}

// IGraphNode
HRESULT Terrain::Update( const float & rSecsDelta )
{
	if( rSecsDelta > 1.0f )
		return S_OK;

	using glm::mat4;
	using glm::vec3;

	// rotate the terrain about the y axis according to arrow key input
	static float rYRotation		= 0.0f;
	static float rScaleFactor	= 0.0f;

	if( InputMgr::Ref().Keybd().Left() == KeyData::KeyDown )
	{
		rYRotation -= 0.5f;
		InputMgr::Ref().Keybd().Left(KeyData::KeyUp);
	}
	if( InputMgr::Ref().Keybd().Right() == KeyData::KeyDown )
	{
		rYRotation += 0.5f;
		InputMgr::Ref().Keybd().Right(KeyData::KeyUp);
	}
	
	mat4 w = glm::rotate( m_Data.W(), rYRotation, vec3(0.0, 1.0, 0.0) );
	
	m_Data.W()		= w;

	m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();	
	
	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() *
						m_Data.Stack();	

	// Updates child nodes
	IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Terrain::PreRender()
{
	if( ! m_pShaderProgram )
		return E_POINTER;

	SetShaderArgs();

	// execute the shader program
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	return S_OK;
}
HRESULT Terrain::Render()
{
	if( ! m_pShaderProgram )
		return E_POINTER;

	glDrawElements( GL_TRIANGLES, m_Plane.IndexCount(), GL_UNSIGNED_INT, ((char*)NULL) );

	return S_OK;
}
HRESULT Terrain::PostRender()
{
	using AntiMatter::AppLog;

	if( ! m_pShaderProgram )
		return E_POINTER;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	AppLog::Ref().OutputGlErrors();
		
	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
	glBindVertexArray(0);
	glUseProgram(0);

	AppLog::Ref().OutputGlErrors();

	return S_OK;
}
HRESULT Terrain::DrawItem()
{
	// execute default behaviour, which renders all child nodes first
	IGraphNode::DrawItem();

	// Render terrain:
	PreRender();
	Render();
	PostRender();

	return S_OK;
}
