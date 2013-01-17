
#include "stdafx.h"
#include "Quad.h"

#include <AntiMatter\AppLog.h>
#include <comdef.h>
#include <glm\glm.hpp>

Quad::Quad() : 
	m_bInitialized	( false ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL ),
	m_rWidth		( 10.0f ),
	m_rHeight		( 10.0f ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 )
{
	m_bInitialized = Initialize();	
}
Quad::Quad( const float rWidth, const float rHeight ): 
	m_bInitialized	( false ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL ),
	m_rWidth		( rWidth ),
	m_rHeight		( rHeight ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 )
{
	m_bInitialized = Initialize();	
}
Quad::Quad( const Quad & r ) : 
	m_bInitialized	( false ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL ),
	m_rWidth		( r.m_rWidth),
	m_rHeight		( r.m_rHeight ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 )
{
	if( r.Initialized() )
	{
		m_bInitialized = Initialize();
	}			
}
Quad & Quad::operator=( const Quad & r )
{	
	if( this != &r )
	{
		if( m_bInitialized )
			Uninitialize();

		m_rWidth	= r.m_rWidth;
		m_rHeight	= r.m_rHeight;

		if( r.Initialized() )
			m_bInitialized = Initialize();	
	}

	return *this;
}
Quad::~Quad()
{
	Uninitialize();
}

bool Quad::Initialize()
{	
	if( m_bInitialized )
		return true;

	using AntiMatter::AppLog;
	using std::string;

	HRESULT hr		= S_OK;
	m_nVertCount	= 4;
	m_nIndexCount	= 6;

	hr = GenerateVertices( &m_pVertices );
	if( ! SUCCEEDED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Quad::Initialize() failed to GenerateVertices(): %s", 
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	hr = GenerateIndices( &m_pdwIndices );
	if( ! SUCCEEDED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Quad::Initialize() failed to GenerateIndices(): %s", 
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	m_bInitialized = true;

	return true;
}
void Quad::Uninitialize()
{
	if( ! m_bInitialized )
		return;

	m_bInitialized = false;

	if( m_pVertices )
	{
		delete [] m_pVertices;
		m_pVertices = NULL;
	}

	if( m_pdwIndices )
	{
		delete [] m_pdwIndices;
		m_pdwIndices = NULL;
	}

	m_rWidth		= 0.0f;
	m_rHeight		= 0.0f;
	m_nVertCount	= 0;
	m_nIndexCount	= 0;	
}
	
// IGeometry
HRESULT Quad::GenerateVertices( CustomVertex** ppVertices )
{
	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppVertices )
		return E_INVALIDARG;

	*ppVertices = new (std::nothrow) CustomVertex[m_nVertCount];
	if( ! *ppVertices )
		return E_OUTOFMEMORY;

	// vertices

	float rHalfWidth	= m_rWidth / 2.0f;
	float rHalfHeight	= m_rHeight / 2.0f;

	(*ppVertices)[0].m_Position[X] = -rHalfWidth;
	(*ppVertices)[0].m_Position[Y] = 0.0f;
	(*ppVertices)[0].m_Position[Z] = -rHalfHeight;

	(*ppVertices)[1].m_Position[X] = -rHalfWidth;
	(*ppVertices)[1].m_Position[Y] = 0.0f;
	(*ppVertices)[1].m_Position[Z] = rHalfHeight;

	(*ppVertices)[2].m_Position[X] = rHalfWidth;
	(*ppVertices)[2].m_Position[Y] = 0.0f;
	(*ppVertices)[2].m_Position[Z] = -rHalfHeight;

	(*ppVertices)[3].m_Position[X] = rHalfWidth;
	(*ppVertices)[3].m_Position[Y] = 0.0f;
	(*ppVertices)[3].m_Position[Z] = rHalfHeight;

	// texture coords
	(*ppVertices)[0].m_TexCoords[U] = 0.0f;
	(*ppVertices)[0].m_TexCoords[V] = 0.0f;	

	(*ppVertices)[1].m_TexCoords[U] = 0.0f;
	(*ppVertices)[1].m_TexCoords[V] = 1.0f;	

	(*ppVertices)[2].m_TexCoords[U] = 1.0f;
	(*ppVertices)[2].m_TexCoords[V] = 0.0f;

	(*ppVertices)[3].m_TexCoords[U] = 1.0f;
	(*ppVertices)[3].m_TexCoords[V] = 1.0f;	

	// normals	
	// per vertex colours ( all set white )
	for( int n = 0; n < 4; n ++ )
	{
		(*ppVertices)[n].m_Normal[X] = 0.0f;
		(*ppVertices)[n].m_Normal[Y] = 1.0f;
		(*ppVertices)[n].m_Normal[Z] = 0.0f;

		(*ppVertices)[n].m_Colour[R] = 1.0f;
		(*ppVertices)[n].m_Colour[G] = 1.0f;
		(*ppVertices)[n].m_Colour[B] = 1.0f;
		(*ppVertices)[n].m_Colour[A] = 1.0f;
	}

	/*
	// tangent
	float x1 = (*ppVertices)[1].m_Position[X] - (*ppVertices)[0].m_Position[X];
	float y1 = (*ppVertices)[1].m_Position[Y] - (*ppVertices)[0].m_Position[Y];
	float z1 = (*ppVertices)[1].m_Position[Z] - (*ppVertices)[0].m_Position[Z];
	float u1 = (*ppVertices)[1].m_TexCoords[U] - (*ppVertices)[0].m_TexCoords[U];    

	using glm::vec3;
	using glm::normalize;
	using glm::cross;

	vec3 vTangent = normalize(vec3( x1/u1, y1/u1, z1/u1 ));
	(*ppVertices)[0].m_Tangent[X] = vTangent.x;
	(*ppVertices)[0].m_Tangent[Y] = vTangent.y;
	(*ppVertices)[0].m_Tangent[Z] = vTangent.z;

	vec3 vNormal	= vec3( (*ppVertices)[0].m_Normal[X], (*ppVertices)[0].m_Normal[Y], (*ppVertices)[0].m_Normal[Z] );	
	vec3 vBiTangent = normalize(cross(vNormal, vTangent));
	

	(*ppVertices)[0].m_Bitangent[X] = vBiTangent.x;
	(*ppVertices)[0].m_Bitangent[Y] = vBiTangent.y;
	(*ppVertices)[0].m_Bitangent[Z] = vBiTangent.z;

	(*ppVertices)[1].m_Bitangent[X] = vBiTangent.x;
	(*ppVertices)[1].m_Bitangent[Y] = vBiTangent.y;
	(*ppVertices)[1].m_Bitangent[Z] = vBiTangent.z;

	(*ppVertices)[2].m_Bitangent[X] = vBiTangent.x;
	(*ppVertices)[2].m_Bitangent[Y] = vBiTangent.y;
	(*ppVertices)[2].m_Bitangent[Z] = vBiTangent.z;
    

    m_vertices[0]->m_bitangent=new Vec3<float>(Vec3Cross(m_vertices[0]->m_normal, m_vertices[0]->m_tangent)->Normalize());
    m_vertices[1]->m_bitangent=new Vec3<float>(Vec3Cross(m_vertices[1]->m_normal, m_vertices[1]->m_tangent)->Normalize());
    m_vertices[2]->m_bitangent=new Vec3<float>(Vec3Cross(m_vertices[2]->m_normal, m_vertices[2]->m_tangent)->Normalize());


	// bitangent
	*/
	
	return S_OK;
}
HRESULT Quad::GenerateIndices( unsigned short** ppIndices )
{
	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppIndices )
		return E_INVALIDARG;

	*ppIndices = new (std::nothrow) unsigned short[m_nIndexCount];
	if( ! *ppIndices )
		return E_OUTOFMEMORY;

	(*ppIndices)[0] = 0;
	(*ppIndices)[1] = 1;
	(*ppIndices)[2] = 2;
	(*ppIndices)[3] = 2;
	(*ppIndices)[4] = 1;
	(*ppIndices)[5] = 3;

	return S_OK;
}
