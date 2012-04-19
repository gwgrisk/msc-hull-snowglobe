
#pragma once
#include "stdafx.h"

#include "Globe.h"
#include "IGraphNode.h"
#include "IGeometry.h"
#include "SceneGraph.h"
#include "VertexClass.h"
#include "Sphere.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "ShaderProgram.h"

#include "slowglobe-cfg.h"

#include <string>
#include <glm\glm.hpp>
#include <AntiMatter\AppLog.h>
#include <AntiMatter\ShellUtils.h>
#include <AntiMatter\constants.h>


Globe::Globe() : 
	IGraphNode			( NULL, NULL, std::string("globe") ),
	m_bInitialized		( false ),
	m_pShaderProgram	( NULL ),	
	m_rRadius			( 10.0f ),
	m_nStacks			( 10 ),
	m_nSlices			( 10 ),
	m_rRotYAngle		( 30.0f ),
	m_rRotYStep			( 4.0f ),
	m_rRotYSpeed		( 3.0f )
{
	m_bInitialized = Initialize();
}
Globe::Globe(	SceneGraph * pGraph, IGraphNode* pParent, const std::string & sId, 
				float rRadius, int nSlices, int nStacks, 
				const std::string & sTexture,
				const glm::vec4 & vColour ) : 
	IGraphNode			( pGraph, pParent, sId ),
	m_bInitialized		( false ),
	m_pShaderProgram	( NULL ),
	m_sTexture			( sTexture ),
	m_rRadius			( rRadius ),
	m_nStacks			( nStacks ),
	m_nSlices			( nSlices ),
	m_rRotYAngle		( 30.0f ),
	m_rRotYStep			( 4.0f ),
	m_rRotYSpeed		( 3.0f ),
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
bool Globe::SetTexture()
{
	using namespace AntiMatter;

	if( m_sTexture.length() == 0 )
	{
		glBindTexture( GL_TEXTURE_2D, 0 );
		return false;
	}

	if( ! Shell::FileExists( m_sTexture ) )
	{
		AppLog::Ref().LogMsg("Globe::Initialize() texture not found");
		return false;
	}
	
	m_tex = Texture( m_sTexture );
	if(!  m_tex.Initialized() )
	{
		AppLog::Ref().LogMsg("Globe::Initialize() texture load failed");
		return false;
	}

	return true;
}

bool Globe::Initialize()
{
	using namespace AntiMatter;
	using glm::vec3;

	if( m_bInitialized )
		return true;

	SetTexture();
	
	// material
	m_material.Ka(vec3(0.1, 0.1, 0.1));
	m_material.Kd(vec3(0.2, 0.2, 0.2));
	m_material.Ks(vec3(0.8, 0.8, 0.8));
	m_material.Shininess( 4.0f );


	m_Sphere = Sphere( m_rRadius, m_nSlices, m_nStacks );
	if( ! m_Sphere.Initialized() )
	{
		AppLog::Ref().LogMsg("Globe::Initialize() failed, geometry primitive Sphere didn't initialize properly");
		return false;
	}

	SetPerVertexColour();	// can only do this after the sphere has been created

	if( ! CreateShader() )
	{
		AppLog::Ref().LogMsg("Globe::Initialize() failed, CreateShader failed for this object");
		return false;
	}

	m_bInitialized = true;
	return m_bInitialized;
}
void Globe::Uninitialize()
{
	m_Sphere.Uninitialize();

	if( m_pShaderProgram )
	{
		delete m_pShaderProgram;
		m_pShaderProgram = NULL;
	}

	m_bInitialized = false;
}

bool Globe::CreateShader()
{
	using namespace std;
	using namespace AntiMatter;

	// Create a ShaderProgram object and initialize it for this object
	// This is a candidate for a factory object / scripting
	vector<ShaderDesc>				vDescs;
	vector<ShaderInputAttribute>	vArgs;
	ShaderDesc						VertShaderDesc;
	ShaderDesc						PixShaderDesc;
	ShaderInputAttribute			VertPosArg;
	ShaderInputAttribute			VertNormalArg;
	ShaderInputAttribute			VertColourArg;

	VertShaderDesc.sFileName	= g_Cfg.ShadersDir() + string("x-ray.vert");		
	PixShaderDesc.sFileName		= g_Cfg.ShadersDir() + string("x-ray.frag");
	//VertShaderDesc.sFileName	= g_Cfg.ShadersDir() + string("ml-ads.vert");		
	//PixShaderDesc.sFileName	= g_Cfg.ShadersDir() + string("ml-ads.frag");

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

	VertColourArg.sFieldName	= string("VertexColor");
	VertColourArg.nFieldSize	= 4;
	VertColourArg.nFieldOffset	= 6;
	VertColourArg.nStride		= sizeof(CustomVertex);	

	vDescs.push_back ( VertShaderDesc );
	vDescs.push_back ( PixShaderDesc );

	vArgs.push_back ( VertPosArg );
	vArgs.push_back ( VertNormalArg );
	vArgs.push_back ( VertColourArg );
	

	m_pShaderProgram = new ShaderProgram( m_Sphere.Vertices(), m_Sphere.Indices(), m_Sphere.VertCount(), m_Sphere.IndexCount(), vDescs, vArgs );
	if( ! m_pShaderProgram->Initialized() )
	{
		delete m_pShaderProgram;
		m_pShaderProgram = NULL;
		return false;
	}

	return true;
}
void Globe::SetShaderArgs()
{
	using namespace AntiMatter;
	using namespace glm;
	using std::string;

	// Indicate which ShaderProgram to use
	glUseProgram( m_pShaderProgram->ShaderProgId() );

	// bind buffers		
	glBindVertexArray( m_pShaderProgram->VaoId() );
	glBindBuffer( GL_ARRAY_BUFFER, m_pShaderProgram->VboId() );	

	// ensure no texture selected (might've left one selected from previous object)
	glBindTexture( GL_TEXTURE_2D, 0 );	

	// Assign uniform variables	
	glm::mat4 mView			= this->Graph()->Cam().V();
	glm::mat4 mModel		= m_Data.W();
	glm::mat4 mModelView	= mView * mModel;

	glm::mat3 mNormal(mModelView);		
	mNormal = glm::transpose(mNormal._inverse());
	
	m_pShaderProgram->AssignUniformFloat( string("rFallOff"),	0.995f );
	m_pShaderProgram->AssignUniformMat4( string("mModelView"),	mModelView );	
	m_pShaderProgram->AssignUniformMat4( string("mMVP"),		m_Data.MVP() );
	m_pShaderProgram->AssignUniformMat3( string("mNormal"),		mNormal );

	AppLog::Ref().OutputGlErrors();
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

	if( ! m_pShaderProgram )
		return E_POINTER;

	using namespace AntiMatter;
	
	if( m_tex.Initialized() )
		glBindTexture( GL_TEXTURE_2D, m_tex.TextureId() );
	else
		glBindTexture( GL_TEXTURE_2D, 0 );
	

	glDisable( GL_DEPTH_TEST );
	
	SetShaderArgs();
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