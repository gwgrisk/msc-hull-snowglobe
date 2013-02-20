
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
#include "EffectMgr.h"

#include "VertexClass.h"
#include "SeasonalTimeline.h"
#include "slowglobe-cfg.h"
#include <vector>
#include <sstream>
	
	
Light::Light() :
	IGraphNode		( NULL, NULL, std::string("light") ),
	m_pEffect		( NULL ),
	m_pVbo			( NULL ),
	m_nVaoId		( 0 ),	
	m_vPosition		( glm::vec4(0.0, 0.0, 0.0, 0.0) ),
	m_La			( glm::vec3(0.5, 0.5, 0.5) ),
	m_Ld			( glm::vec3(0.8, 0.8, 0.8) ),
	m_Ls			( glm::vec3(0.2, 0.2, 0.2) ),
	m_Type			( LightType::DirectionalLight ),
	m_vDirection	( glm::vec3(0.0, 0.0, 0.0) ),
	m_rExponent		( 1.0f ),
	m_rCutOff		( 20.0f )
{
	Initialize();
}
Light::Light(
		SceneGraph *		pGraph, 
		IGraphNode*			pParent, 
		const std::string & sId, 
		const glm::vec4 &	pos, 
		const glm::vec3 &	La, 
		const glm::vec3 &	Ld, 
		const glm::vec3 &	Ls,
		const LightType		lt,
		const glm::vec3 &	vDir, 
		const real			exp, 
		const real			rCutoff 
	) :
	IGraphNode		( pGraph, pParent, sId ),
	m_pEffect		( NULL ),
	m_pVbo			( NULL ),
	m_nVaoId		( 0 ),	
	m_vPosition		( pos ),
	m_La			( La ),
	m_Ld			( Ld ),
	m_Ls			( Ls ),
	m_Type			( lt ),
	m_vDirection	( vDir ),
	m_rExponent		( exp ),
	m_rCutOff		( rCutoff )
{
	Initialize();
}
Light::Light( const Light & rhs ) :
	IGraphNode		( rhs.m_pGraph, rhs.m_pParent, rhs.m_sId ),
	m_pEffect		( NULL ),
	m_pVbo			( NULL ),
	m_nVaoId		( 0 ),	
	m_vPosition		( rhs.m_vPosition ),
	m_La			( rhs.m_La ),
	m_Ld			( rhs.m_Ld ),
	m_Ls			( rhs.m_Ls ),
	m_Type			( rhs.m_Type ),	
	m_vDirection	( rhs.m_vDirection ),
	m_rExponent		( rhs.m_rExponent ),
	m_rCutOff		( rhs.m_rCutOff )
{
	Initialize();
}
Light & Light::operator=( const Light & rhs )
{
	if( this != &rhs )
	{	
		Uninitialize();

		// IGraphNode
		m_pGraph		= rhs.m_pGraph;
		m_pParent		= rhs.m_pParent;
		m_sId			= rhs.m_sId;

		// Light
		m_material		= rhs.m_material;
		m_vPosition		= rhs.m_vPosition;	
		m_La			= rhs.m_La;
		m_Ld			= rhs.m_Ld;
		m_Ls			= rhs.m_Ls;
		m_Type			= rhs.m_Type;
		m_vDirection	= rhs.m_vDirection;
		m_rExponent		= rhs.m_rExponent;
		m_rCutOff		= rhs.m_rCutOff;

		if( m_pEffect )
			m_pEffect = NULL;

		Initialize();
	}

	return *this;
}
	
void Light::Initialize()
{
	using namespace AntiMatter;
	using glm::vec3;

	glex::Load();
	
	InitializeMaterial();

	if( ! InitializeGeometry() )
	{
		AppLog::Ref().LogMsg("%s initialize geometry failed for globe object", __FUNCTION__ );
		return;
	}

	if( ! GetShader() )
	{
		AppLog::Ref().LogMsg( "%s effect not available", __FUNCTION__ );
		Uninitialize();
		return;
	}

	if( ! InitializeVbo( m_Sphere ) )
	{
		AppLog::Ref().LogMsg( "%s failed to Create a vertex buffer from supplied geometry", __FUNCTION__ );
		Uninitialize();
		return;
	}

	if( ! InitializeVao() )
	{
		AppLog::Ref().LogMsg( "%s failed to Create a vertex array object", __FUNCTION__ );
		Uninitialize();
		return;
	}
	
	if( ! SetPerVertexColour() )	// can only do this after the geometry has been created
	{
		AppLog::Ref().LogMsg("%s initialize vertex colours failed for globe object", __FUNCTION__ );
		return;
	}			
}
void Light::Uninitialize()
{
	// geometry	
	m_Sphere.Uninitialize();
	
	// material (not necessary)
	
	// Shader
	m_pEffect = NULL;

	if( m_pVbo )
	{
		delete m_pVbo;
		m_pVbo = NULL;
	}
}

bool Light::InitializeGeometry()
{
	using AntiMatter::AppLog;

	m_Sphere = Sphere( 18.0f, 6, 6 );

	if( ! m_Sphere.Initialized() )
	{
		AppLog::Ref().LogMsg("%s failed, geometry primitive Sphere didn't initialize properly", __FUNCTION__);
		assert(0);
		return false;
	}

	return true;
}
void Light::InitializeMaterial()
{
	// light ball colours are taken from the m_material definition

	using glm::vec3;	

	if( this->Id().compare("light0") == 0 )			// red material
	{
		m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
		m_material.Kd(vec3(1.0f, 0.0f, 0.0f));
		m_material.Ks(vec3(1.0f, 0.0f, 0.0f));
		m_material.Shininess( 1.0f );		
	}
	else if( this->Id().compare("light1") == 0 )	// yellow mtl
	{
		m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
		m_material.Kd(vec3(1.0f, 1.0f, 0.0f));
		m_material.Ks(vec3(1.0f, 1.0f, 0.0f));
		m_material.Shininess( 1.0f );
	}
	else if( this->Id().compare("light2") == 0 )	// green mtl
	{
		m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
		m_material.Kd(vec3(0.0f, 1.0f, 0.0f));
		m_material.Ks(vec3(0.0f, 1.0f, 0.0f));
		m_material.Shininess( 1.0f );
	}
	else if( this->Id().compare("light3") == 0 )	// blue mtl
	{
		m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
		m_material.Kd(vec3(0.0f, 0.0f, 1.0f));
		m_material.Ks(vec3(0.0f, 0.0f, 1.0f));
		m_material.Shininess( 1.0f );
	}
	else if( this->Id().compare("sun") == 0 )		// white mtl
	{
		m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
		m_material.Kd(vec3(1.0f, 1.0f, 1.0f));
		m_material.Ks(vec3(1.0f, 1.0f, 1.0f));
		m_material.Shininess( 1.0f );
	}	
	else if( this->Id().compare("globe-specular") == 0 )	// white mtl
	{
		m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
		m_material.Kd(vec3(1.0f, 1.0f, 1.0f));
		m_material.Ks(vec3(1.0f, 1.0f, 1.0f));
		m_material.Shininess( 1.0f );
	}
	else
	{
		m_material.Ka(vec3(0.1f, 0.1f, 0.1f));
		m_material.Kd(vec3(1.0f, 1.0f, 1.0f));
		m_material.Ks(vec3(1.0f, 1.0f, 1.0f));
		m_material.Shininess( 1.0f );
	}	
}
bool Light::InitializePosition()
{
	using glm::mat4;
	using glm::translate;
	using glm::vec3;

	bool bInitialLightPosSet = false;

	// initialize sphere position
	if( m_Sphere.Initialized() && ! bInitialLightPosSet && m_pParent )
	{			
		mat4 tr			= translate( mat4(1.0), vec3(m_vPosition) );
		m_Data.W()		= tr;

		m_Data.Stack()	= m_Data.W();

		m_Data.MVP() =	
			m_pGraph->Proj().P() * 
			m_pGraph->Cam().V() * 
			m_Data.Stack();		

		bInitialLightPosSet = true;
	}

	return bInitialLightPosSet;
}
bool Light::SetPerVertexColour()
{
	// copies the specified colour to each vertex
	CustomVertex* pVertices  =*(m_Sphere.Vertices());
	
	for( int n = 0; n < m_Sphere.VertCount(); n ++ )
	{		
		pVertices[n].m_Colour[0] = m_material.Kd().r;
		pVertices[n].m_Colour[1] = m_material.Kd().g;
		pVertices[n].m_Colour[2] = m_material.Kd().b;
		pVertices[n].m_Colour[3] = 1.0f;
	}
	
	return true;
}

bool Light::InitializeVbo( IGeometry & geometry )
{
	using AntiMatter::AppLog;

	m_pVbo = new (std::nothrow) Vbo<CustomVertex> ( 
		geometry.VertCount(), 
		geometry.Vertices(), 
		geometry.IndexCount(), 
		geometry.Indices() 
	);

	if( (m_pVbo == NULL) || (! m_pVbo->Initialized()) )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize vertex buffer for object geometry", __FUNCTION__ );
		assert(0);
		return false;
	}

	return true;
}
bool Light::InitializeVao()
{
	using AntiMatter::AppLog;

	if( ! m_pEffect )
	{
		assert(0);
		return false;
	}

	glUseProgram( m_pEffect->Id() );

	glGenVertexArrays( 1, &m_nVaoId );
	glBindVertexArray( m_nVaoId );	

	glBindBuffer( GL_ARRAY_BUFFER,			m_pVbo->Id() );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	m_pVbo->IndexId() );
	
	// set the VBO attribute pointers
	// you'll find the attributes for the effect defined in the 
	// xml file that defines the effect.
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

bool Light::GetShader()
{
	using std::string;
	bool bRet = false;

// warning C4482: nonstandard extension used: enum 'Effect::EffectBuildState' used in qualified name
#pragma warning (push)
#pragma warning (disable: 4482)
	if( EffectMgr::Ref().Find( string("flat"), &m_pEffect ) )
	{
		if( m_pEffect->BuildState() == Effect::EffectBuildState::Linked )						
			bRet = true;
	}
#pragma warning (pop)

	return bRet;
}
bool Light::SetUniforms()
{	
	using std::string;

	if( ! m_pEffect )
		return false;

	// Indicate which ShaderProgram to use
	glUseProgram( m_pEffect->Id() );
	
	// bind buffers		
	glBindVertexArray( m_nVaoId );
	glBindBuffer( GL_ARRAY_BUFFER, m_pVbo->Id() );
	
	m_pEffect->AssignUniformMat4( string("mMVP"), m_Data.MVP() );

	return true;
}


// IGraphNode
HRESULT Light::Update( const float & rSecsDelta )
{	
	// we only need to update the position of the sun

	// Updates child nodes
	// Lights don't have any children, so we can skip this step
	//if( IGraphNode::HasChildNodes() )
	//IGraphNode::UpdateChildren( rSecsDelta );

	using glm::mat4;
	using glm::vec3;
	using glm::translate;
	using glm::rotate;

	if( rSecsDelta > 1.0f )
		return S_OK;

	if( this->Id().compare("sun") == 0 )
	{
		static float	rAngle = 0.0f;
		float			rSpeed = SeasonalTimeline::Ref().Speed();

		rAngle = rSecsDelta * 30.0f * rSpeed;

		// initialize sphere position
		if( m_Sphere.Initialized() )
		{
			mat4 rot		= glm::rotate( mat4(1.0f), rAngle, glm::vec3(0.0, 0.0, 1.0) );
			mat4 tr			= translate( mat4(1.0), vec3(m_vPosition) );
			m_Data.W()		=  rot * tr;

			m_Data.Stack()	= m_Data.W();

			m_Data.MVP() =	
				m_pGraph->Proj().P() * 
				m_pGraph->Cam().V() * 
				m_Data.Stack();

			m_vPosition = glm::vec4( m_Data.W()[3] );
		}
	}
	else
	{
		InitializePosition();
	}

	
	return S_OK;
}
HRESULT Light::PreRender()
{
	if( ! m_pEffect )
		return E_POINTER;

	SetUniforms();
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
		GL_UNSIGNED_SHORT, 
		m_Sphere.Indices()
	);

	// stacks
	for( int n = 0; n < nStacks-2; n ++ )
	{
		glDrawElements(
			GL_TRIANGLE_STRIP,
			((nSlices+1)*2),
			GL_UNSIGNED_SHORT,
			&(m_Sphere.Indices())[ nSlices+2+n*(nSlices+1)*2 ]
		);
	}

	// bottom triangle fan
	glDrawElements( 
		GL_TRIANGLE_FAN, 
		nSlices+2,
		GL_UNSIGNED_SHORT, 
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
	out << "id "		<< r.Id()	 << "\n";	
	out << "position "	<< r.Pos().x		<< " " << r.Pos().y			<< " " << r.Pos().z			<< " "	<< r.Pos().w << "\n";
	out << "La "		<< r.La().x	 << " " << r.La().y	<< " "	<< r.La().z << "\n";
	out << "Ld "		<< r.Ld().x	 << " " << r.Ld().y	<< " "	<< r.Ld().z << "\n";
	out << "Ls "		<< r.Ls().x	 << " " << r.Ls().y	<< " "	<< r.Ls().z << "\n" << std::endl;
	out << "type "		<< r.Type()  << "\n";
	out << "direction "	<< r.Direction().x	<< " " << r.Direction().y	<< " " << r.Direction().z	<< "\n";
	out << "exponent "	<< r.Exponent() << "\n";
	out << "cutoff "	<< r.CutOff() << "\n";

	return out;
}
std::istream & operator >> ( std::istream & in, Light & r )
{
	using AntiMatter::AppLog;
	using std::string;
	using glm::vec3;
	using glm::vec4;

	string	sBuffer;
	string	sId;
	
	vec4	tmpPos;	
	vec3	la, ld, ls;	
	int		tmpType;	
	vec3	tmpDirection;
	real	rExp, rCutOff;

	try
	{
		in >> sBuffer >> sId;
		in >> sBuffer >> tmpPos.x >> tmpPos.y >> tmpPos.z	>> tmpPos.w;		
		in >> sBuffer >> la.x >> la.y	>> la.z;
		in >> sBuffer >> ld.x >> ld.y	>> ld.z;
		in >> sBuffer >> ls.x >> ls.y	>> ls.z;
		in >> sBuffer >> tmpType;		
		in >> sBuffer >> tmpDirection.x >> tmpDirection.y >> tmpDirection.z;
		in >> sBuffer >> rExp;
		in >> sBuffer >> rCutOff;
			
		r.SetId( sId );
		r.Pos( tmpPos );
		r.La( la );
		r.Ld( ld );
		r.Ls( ls );
		r.Type( (Light::LightType)tmpType );
		r.Direction( tmpDirection );
		r.Exponent( rExp );
		r.CutOff( rCutOff );
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated (probably disk) exception, or invalid light config file", __FUNCTION__ );
	}	

	return in;
}