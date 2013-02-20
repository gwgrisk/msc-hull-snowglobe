
#include "stdafx.h"
#include "Terrain.h"
#include "Plane.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"

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
#include "EffectMgr.h"
#include "Effect.h"

Terrain::Terrain() :
	IGraphNode			( NULL, NULL, std::string("terrain") ),
	m_bInitialized		( false ),
	m_nTextureId		( 0 ),
	m_nVertCols			( 0 ),
	m_nVertRows			( 0 ),	
	m_rCellWidth		( 0.0f ),
	m_rCellHeight		( 0.0f ),
	m_rCellDepth		( 0.0f ),
	m_pEffect			( NULL ),
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
	m_pEffect			( NULL ),
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
	m_pEffect			( NULL ),
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
	m_pEffect			( NULL ),
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
	using namespace AntiMatter;
	using namespace std;
	using glm::vec3;

	glex::Load();

	m_bOldShader = false;

	if( (m_nVertRows == 0) || (m_nVertCols == 0) || 
		(fabs(m_rCellWidth) == 0.0f) || (fabs(m_rCellHeight) == 0.0f) || (fabs(m_rCellDepth) == 0.0f) )
			return false;	

	
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

	if( ! GetShader() )
	{
		Uninitialize();
		AppLog::Ref().LogMsg("%s failed to initialize shaders for the tree", __FUNCTION__);
		return false;
	}

	if( ! InitializeVbo(m_Plane) )
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
	
	m_bInitialized = true;

	return m_bInitialized;
}
bool Terrain::InitializeMtl()
{
	using glm::vec3;

	// material
	m_material.Ka(vec3(0.0, 0.0, 0.0));
	m_material.Kd(vec3(0.2, 1.0, 0.2));
	m_material.Ks(vec3(0.0, 0.1, 0.0));
	m_material.Shininess( 1.0f );

	return true;
}
bool Terrain::InitializeGeometry()
{
	using AntiMatter::AppLog;	

	m_Plane = Plane(m_nVertCols, m_nVertRows, m_rCellWidth, m_rCellHeight, m_rCellDepth);
	ApplyHeightField();

	if( ! m_Plane.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed.", __FUNCTION__ );
		return false;
	}

	return true;
}
bool Terrain::InitializeTextures()
{
	using AntiMatter::Shell::FileExists;

	// load texture
	if( FileExists( m_sTexture ) )
		m_texGrass = Texture( m_sTexture.c_str(), false );

	// load alpha map
	if( FileExists( m_sAlphaMap ) )
		m_texAlphaMap = Texture( m_sAlphaMap );

	return (m_texGrass.Initialized() && m_texAlphaMap.Initialized());
}
bool Terrain::InitializeVbo( IGeometry & geometry )
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
bool Terrain::InitializeVao()
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

	
bool Terrain::GetShader()
{
	using std::string;

	// warning C4482: nonstandard extension used: enum 'Effect::EffectBuildState' used in qualified name
#pragma warning (push)
#pragma warning (disable: 4482)
	if( EffectMgr::Ref().Find( string("textured-phong-alphamap"), &m_pEffect ) )
	{
		if( m_pEffect->BuildState() == Effect::EffectBuildState::Linked )					
		{
			m_nSunSub		= glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Sunlight" );
			m_nSpotlightSub = glGetSubroutineIndex( m_pEffect->Id(), GL_FRAGMENT_SHADER, "Spotlights" );
			return true;		
		}
	}
#pragma warning (pop)

	return false;
}
void Terrain::SetShaderArgs()
{
	using namespace glm;
	using namespace AntiMatter;
	using namespace std;

	// I only put these into variables for debug purposes
	GLuint nShaderProgId	= m_pEffect->Id();
	GLuint nVaoId			= m_nVaoId;
	GLuint nVboId			= m_pVbo->Id();
	GLuint nVboIndexId		= m_pVbo->IndexId();
	GLuint nTexId			= m_texGrass.TextureId();
	GLuint nAlphaId			= m_texAlphaMap.TextureId();

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
	// vec3 vCamPos = vec3(Graph()->Cam().V() * m_Data.W() * vec4( Graph()->Cam().Pos(), 1.0));
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
	AppLog::Ref().OutputGlErrors();

	// textures
	m_pEffect->AssignUniformSampler2D( string("tex"),			nTexId );
	m_pEffect->AssignUniformSampler2D( string("texAlpha"),		nAlphaId );
	AppLog::Ref().OutputGlErrors();
	
	// matrices
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mView * mModel;

	glm::mat3 mNormal(mModelView);
	mNormal = glm::transpose(mNormal._inverse());

	m_pEffect->AssignUniformMat4( string("mModelView"),		mModelView );
	m_pEffect->AssignUniformMat3( string("mNormal"),		mNormal );
	m_pEffect->AssignUniformMat4( string("mMVP"),			m_Data.MVP() );
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
	if( IGraphNode::HasChildNodes() )
		IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Terrain::PreRender()
{
	if( ! m_pEffect )
		return E_POINTER;

	SetShaderArgs();

	// execute the shader program
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	return S_OK;
}
HRESULT Terrain::Render()
{
	if( ! m_pEffect )
		return E_POINTER;

	glDrawElements( GL_TRIANGLES, m_Plane.IndexCount(), GL_UNSIGNED_SHORT, ((char*)NULL) );

	return S_OK;
}
HRESULT Terrain::PostRender()
{
	using AntiMatter::AppLog;

	if( ! m_pEffect )
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
