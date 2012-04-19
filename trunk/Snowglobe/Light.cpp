
#include "stdafx.h"
#include "Light.h"

#include <AntiMatter\AppLog.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <iostream>
#include "SceneGraph.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "ShaderProgram.h"

#include "VertexClass.h"
#include "SeasonalTimeline.h"
#include "slowglobe-cfg.h"
	
	
Light::Light() :
	IGraphNode		( NULL, NULL, std::string("light") ),
	m_pShaderProgram( NULL ),
	m_Type			( DirectionalLight ),
	m_vIntensity	( glm::vec3(0.0, 0.0, 0.0) ),
	m_vPosition		( glm::vec4(0.0, 0.0, 0.0, 0.0) ),
	m_vDirection	( glm::vec3(0.0, 0.0, 0.0) ),
	m_La			( glm::vec3(0.5, 0.5, 0.5) ),
	m_Ld			( glm::vec3(0.8, 0.8, 0.8) ),
	m_Ls			( glm::vec3(0.1, 0.1, 0.1) )
{
	Initialize();
}
Light::Light(	SceneGraph * pGraph, IGraphNode* pParent, const std::string & sId, 
				const glm::vec4 & pos, 
				const glm::vec3 & La, const glm::vec3 & Ld, const glm::vec3 & Ls ) :
	IGraphNode		( pGraph, pParent, sId ),
	m_pShaderProgram( NULL ),
	m_Type			( DirectionalLight ),
	m_vIntensity	( glm::vec3(0.0, 0.0, 0.0) ),
	m_vPosition		( pos ),
	m_vDirection	( glm::vec3(0.0, 0.0, 0.0) ),
	m_La			( La ),
	m_Ld			( Ld ),
	m_Ls			( Ls )
{
	Initialize();
}
Light::Light( const Light & rhs ) :
	IGraphNode		( rhs.m_pGraph, rhs.m_pParent, rhs.m_sId ),
	m_pShaderProgram( NULL ),
	m_Type			( rhs.m_Type ),
	m_vIntensity	( rhs.m_vIntensity ),
	m_vPosition		( rhs.m_vPosition ),
	m_vDirection	( rhs.m_vDirection ),
	m_La			( rhs.m_La ),
	m_Ld			( rhs.m_Ld ),
	m_Ls			( rhs.m_Ls )
{
	Initialize();
}
Light & Light::operator=( const Light & rhs )
{
	if( this != &rhs )
	{	
		// IGraphNode
		m_pGraph		= rhs.m_pGraph;
		m_pParent		= rhs.m_pParent;
		m_sId			= rhs.m_sId;

		// Light
		m_Type			= rhs.m_Type;
		m_vPosition		= rhs.m_vPosition;
		m_vDirection	= rhs.m_vDirection;
		m_vIntensity	= rhs.m_vIntensity;
		m_La			= rhs.m_La;
		m_Ld			= rhs.m_Ld;
		m_Ls			= rhs.m_Ls;

		if( m_pShaderProgram )
		{
			delete m_pShaderProgram;
			m_pShaderProgram = NULL;
		}

		Initialize();
	}

	return *this;
}
	
void Light::Initialize()
{
	using AntiMatter::AppLog;
	using glm::mat4;
	using glm::translate;

	m_Sphere = Sphere( 18.0f, 6, 6 );

	if( ! CreateShader() )	
		AppLog::Ref().LogMsg("%s failed to initialize shader for light sphere", __FUNCTION__ );

}
void Light::Uninitialize()
{
	m_Sphere.Uninitialize();

	if( m_pShaderProgram )
	{
		delete m_pShaderProgram;
		m_pShaderProgram = NULL;
	}
}

bool Light::CreateShader()
{
	using namespace std;
	using namespace AntiMatter;

	// Create a ShaderProgram object and initialize it for this object
	// This is a candidate for a factory object / scripting
	vector<ShaderDesc>	vDescs;
	vector<ShaderInputAttribute>	vArgs;
	ShaderDesc						VertShaderDesc;
	ShaderDesc						PixShaderDesc;
	ShaderInputAttribute			VertPosArg;
	ShaderInputAttribute			VertNormalArg;
	ShaderInputAttribute			VertColourArg;

	VertShaderDesc.sFileName	= g_Cfg.ShadersDir() + string("x-ray.vert");		
	PixShaderDesc.sFileName		= g_Cfg.ShadersDir() + string("x-ray.frag");	

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
void Light::SetShaderArgs()
{
	using namespace AntiMatter;
	using namespace glm;
	using std::string;

	// Indicate which ShaderProgram to use
	glUseProgram( m_pShaderProgram->ShaderProgId() );

	// bind buffers		
	glBindVertexArray( m_pShaderProgram->VaoId() );
	glBindBuffer( GL_ARRAY_BUFFER, m_pShaderProgram->VboId() );		

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
HRESULT Light::Update( const float & rSecsDelta )
{	
	// Updating light position is okay if we've got one Update() function per class, but
	// since the Update will be different for each light, we'll need some way to 
	// affect each light differently.

	// Updates child nodes
	// Lights don't have any children, so we can skip this step
	//IGraphNode::UpdateChildren( rSecsDelta );

	using glm::mat4;
	using glm::vec3;
	using glm::translate;
	using glm::rotate;

	if( rSecsDelta > 1.0f )
		return S_OK;

	static float	rAngle = 0.0f;
	float			rSpeed = SeasonalTimeline::Ref().Speed();

	rAngle = rSecsDelta * 30.0f * rSpeed;

	// initialize sphere position
	if( m_Sphere.Initialized() )
	{
		mat4 rot		= glm::rotate( mat4(1.0f), rAngle, glm::vec3(0.0, 0.0, 1.0) );
		mat4 tr			= translate( mat4(1.0), vec3(m_vPosition) );
		m_Data.W()		=  rot * tr;

		m_Data.Stack()	= m_pParent->GetNodeData().Stack() * m_Data.W();

		m_Data.MVP() =	
			m_pGraph->Proj().P() * 
			m_pGraph->Cam().V() * 
			m_Data.Stack();

		m_vPosition = glm::vec4( m_Data.W()[3] );
	}

	return S_OK;
}
HRESULT Light::PreRender()
{
	if( ! m_pShaderProgram )
		return E_POINTER;

	SetShaderArgs();
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	return S_OK;
}
HRESULT Light::Render()
{
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
HRESULT Light::PostRender()
{
	using AntiMatter::AppLog;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	AppLog::Ref().OutputGlErrors();

	glBindTexture( GL_TEXTURE_2D,			0 );
	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
	glBindVertexArray(0);
	glUseProgram(0);

	glEnable( GL_DEPTH_TEST );

	AppLog::Ref().OutputGlErrors();
	return S_OK;
}
HRESULT Light::DrawItem()
{
	// execute default behaviour, which renders all child nodes first
	IGraphNode::DrawItem();

	// Render terrain:
	PreRender();
	Render();
	PostRender();

	return S_OK;
}
	
// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const Light & r )
{
	out << r.Id()	 << " ";
	out << r.Type()  << " ";
	out << r.Intensity().x	<< " " << r.Intensity().y	<< " " << r.Intensity().z	<< " ";
	out << r.Pos().x		<< " " << r.Pos().y			<< " " << r.Pos().z			<< " "	<< r.Pos().w << " ";
	out << r.Direction().x	<< " " << r.Direction().y	<< " " << r.Direction().z	<< " ";
	out << r.La().x	 << " " << r.La().y	<< " "	<< r.La().z << " ";
	out << r.Ld().x	 << " " << r.Ld().y	<< " "	<< r.Ld().z << " ";
	out << r.Ls().x	 << " " << r.Ls().y	<< " "	<< r.Ls().z << std::endl;

	return out;
}
std::istream & operator >> ( std::istream & in, Light & r )
{
	using AntiMatter::AppLog;
	using std::string;
	using glm::vec3;
	using glm::vec4;

	string	sId;
	int		tmpType;
	vec3	tmpIntensity;
	vec4	tmpPos;	
	vec3	tmpDirection;
	vec3	la, ld, ls;	

	try
	{
		in >> sId;
		in >> tmpType;
		in >> tmpIntensity.x >> tmpIntensity.y >> tmpIntensity.z;
		in >> tmpPos.x >> tmpPos.y >> tmpPos.z	>> tmpPos.w;
		in >> tmpDirection.x >> tmpDirection.y >> tmpDirection.z;
		in >> la.x >> la.y	>> la.z;
		in >> ld.x >> ld.y	>> ld.z;
		in >> ls.x >> ls.y	>> ls.z;
	
		r.Type( (Light::LightType)tmpType );
		r.SetId( sId );
		r.Intensity( tmpIntensity );
		r.Pos( tmpPos );
		r.Direction( tmpDirection );
		r.La( la );
		r.Ld( ld );
		r.Ls( ls );	
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated (probably disk) exception, or invalid light config file", __FUNCTION__ );
	}	

	return in;
}