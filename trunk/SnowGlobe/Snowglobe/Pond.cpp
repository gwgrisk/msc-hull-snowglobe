
#include "stdafx.h"
#include "Pond.h"
#include "IGraphNode.h"

#include "Plane.h"

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

Pond::Pond() :
	IGraphNode			( NULL, NULL, std::string("pond") ),
	m_bInitialized		(false),
	m_pShaderProgram	( NULL ),
	m_rWidth			( 0.0f ),
	m_rHeight			( 0.0f )
{
}
Pond::Pond(	SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, 
		const float rWidth, const float rHeight, 
		const std::string & sTexture,
		const std::string & sAlphaMap,
		const std::string & sNormalMap ) :
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		(false),
	m_pShaderProgram	( NULL ),
	m_sTextureMap		( sTexture ),
	m_sAlphaMap			( sAlphaMap ),
	m_sNormalMap		( sNormalMap ),
	m_rWidth			( rWidth ),
	m_rHeight			( rHeight )
{
	Initialize();
}
Pond::Pond( const Pond & r ) : 
	IGraphNode		( r.m_pGraph, r.m_pParent, r.m_sId ),
	m_bInitialized	( false ),
	m_pShaderProgram( NULL ),
	m_sTextureMap	( r.m_sTextureMap ),
	m_sAlphaMap		( r.m_sAlphaMap ),
	m_sNormalMap	( r.m_sNormalMap ),
	m_rWidth		( r.m_rWidth ),
	m_rHeight		( r.m_rHeight )
{	
	if( r.Initialized() )
		Initialize();
}
Pond & Pond::operator=( const Pond & r )
{
	if( this != &r )
	{
		m_pGraph		= r.m_pGraph;
		m_pParent		= r.m_pParent;
		m_sId			= r.m_sId;

		m_sTextureMap	= r.m_sTextureMap;
		m_sAlphaMap		= r.m_sAlphaMap;
		m_sNormalMap	= r.m_sNormalMap;
		m_rWidth		= r.m_rWidth;
		m_rHeight		= r.m_rHeight;

		if( r.Initialized() )
			Initialize();
	}

	return *this;
}
Pond::~Pond()
{
	Uninitialize();
}

bool Pond::Initialize()
{
	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using glm::vec3;

	glex::Load();
	
	float rCellWidth	= 5.0f;
	float rCellHeight	= 5.0f;
	float rCellDepth	= 0.1f;

	m_Plane = Plane( 
		(int)(m_rWidth/rCellWidth), (int)(m_rHeight/rCellHeight),
		rCellWidth, 
		rCellHeight, 
		rCellDepth 
	);
	if( ! m_Plane.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed to initialize plane geometry", __FUNCTION__ );
		return false;
	}

	m_Quad = Quad( m_rWidth, m_rHeight );
	if( ! m_Quad.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed to initialize quad geometry", __FUNCTION__ );
		return false;
	}

	// material
	m_material.Ka(vec3(0.2, 0.2, 0.2));
	m_material.Kd(vec3(0.5, 0.5, 0.5));
	m_material.Ks(vec3(0.4, 0.4, 0.4));
	m_material.Shininess( 4.0f );

	if( FileExists( m_sTextureMap ) )
		m_TextureMap = Texture( m_sTextureMap );

	if( ! m_TextureMap.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed to load texture map", __FUNCTION__);
		Uninitialize();
		return false;
	}
	
	// initialize alphamap texture
	if( FileExists(m_sAlphaMap) )	
		m_AlphaMap = Texture( m_sAlphaMap );
	
	if( ! m_AlphaMap.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed to load alpha map", __FUNCTION__);
		Uninitialize();
		return false;
	}

	// create shader
	if( ! CreateShader( &m_pShaderProgram, m_Quad ) )
	{
		AppLog::Ref().LogMsg("%s failed to Create a shader", __FUNCTION__);
		Uninitialize();
		return false;
	}

	m_bInitialized = true;

	return m_bInitialized;
}
void Pond::Uninitialize()
{
	m_bInitialized = false;

	m_Plane.Uninitialize();
	
	m_sAlphaMap.clear();
	m_sNormalMap.clear();

	m_AlphaMap = Texture();
}

bool Pond::CreateShader( ShaderProgram** ppShader, IGeometry & pShape )
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
	ShaderInputAttribute			VertTextureArg;
	ShaderInputAttribute			VertNormalArg;
	ShaderInputAttribute			VertColorArg;	
	
	VertShaderDesc.sFileName		= g_Cfg.ShadersDir() + string("textured-phong.vert");
	PixShaderDesc.sFileName			= g_Cfg.ShadersDir() + string("textured-phong-alphamap.frag");
	

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

	if( ! (*ppShader)->Initialized() )
	{
		delete *ppShader;
		*ppShader = NULL;
		return false;
	}

	return true;
}
void Pond::SetShaderArgs( ShaderProgram* pShader, Texture* pTex, Texture* pAlpha, const Material & mat )
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
	GLuint nAlphaId			= pAlpha->TextureId();

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
		glm::vec4 lightPos	= lights[x]->Pos();
		glm::vec3 la		= lights[x]->La();
		glm::vec3 ld		= lights[x]->Ld();
		glm::vec3 ls		= lights[x]->Ls();

		glm::vec4 eyeLightPos = Graph()->Cam().V() * m_Data.W() * lightPos;

		stringstream ssP, ssLa, ssLd, ssLs;			
			
		ssP  << "lights[" << x << "].Position";
		ssLa << "lights[" << x << "].La";
		ssLd << "lights[" << x << "].Ld";
		ssLs << "lights[" << x << "].Ls";

		string sPosition	= ssP.str();
		string sLa			= ssLa.str();
		string sLd			= ssLd.str();
		string sLs			= ssLs.str();

		pShader->AssignUniformVec4( sPosition,	 eyeLightPos );
		pShader->AssignUniformVec3( sLa, la );
		pShader->AssignUniformVec3( sLd, ld );
		pShader->AssignUniformVec3( sLs, ls );
	}
	AppLog::Ref().OutputGlErrors();

	// textures				
	pShader->AssignUniformSampler2D( string("tex"),			nTexId );		
	pShader->AssignUniformSampler2D( string("texAlpha"),	nAlphaId );
	AppLog::Ref().OutputGlErrors();

	// Material
	pShader->AssignUniformVec3(  string("Ka"),			mat.Ka() );
	pShader->AssignUniformVec3(  string("Kd"),			mat.Kd() );
	pShader->AssignUniformVec3(  string("Ks"),			mat.Ks() );
	pShader->AssignUniformFloat( string("rShininess"),	mat.Shininess() );		
	AppLog::Ref().OutputGlErrors();

	// set MVP matrices
	// matrices
	/*
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mView * mModel;
	*/
	glm::mat4 mModelView	= m_pGraph->Cam().V() * m_Data.W();

	glm::mat3 mNormal(mModelView);
	mNormal = glm::transpose(mNormal._inverse());

	pShader->AssignUniformMat4( string("mModelView"),	mModelView );
	pShader->AssignUniformMat3( string("mNormal"),		mNormal );
	pShader->AssignUniformMat4( string("mMVP"),			m_Data.MVP() );

}

// IGraphNode
HRESULT Pond::Update( const float & rSecsDelta )
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
HRESULT Pond::PreRender()
{
	if( ! m_pShaderProgram )
		return E_POINTER;

	// need to disable depth test for transparent objects
	// but this has consequences for the alphamap, so this isn't a viable approach

	SetShaderArgs( m_pShaderProgram, &m_TextureMap, &m_AlphaMap, m_material );

	// execute the shader program
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );

	return S_OK;
}
HRESULT Pond::Render()
{
	if( ! m_pShaderProgram )
		return E_POINTER;

	glDrawElements( GL_TRIANGLES, m_Plane.IndexCount(), GL_UNSIGNED_INT, ((char*)NULL) );

	return S_OK;
}
HRESULT Pond::PostRender()
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
HRESULT Pond::DrawItem()
{
	// execute default behaviour, which renders all child nodes first
	IGraphNode::DrawItem();

	// Render
	PreRender();
	Render();
	PostRender();

	return S_OK;
}