
#include "stdafx.h"

#include "Cylinder.h"
#include "IGraphNode.h"
#include "IGeometry.h"
#include "SceneGraph.h"
#include "VertexClass.h"

#include <string>
#include <glm\glm.hpp>
#include <comdef.h>
#include <AntiMatter\AppLog.h>
#include <AntiMatter\constants.h>


Cylinder::Cylinder() :
	m_bInitialized	( false ),
	m_nStacks		( 0 ),
	m_nSlices		( 0 ),
	m_rStackHeight	( 0.0f ),
	m_rBaseRadius	( 0.0f ),
	m_rTipRadius	( 0.0f ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 ),
	m_nCellCount	( 0 ),
	m_nTriCount		( 0 )
{
}
Cylinder::Cylinder( const int nSlices, const int nStacks, const float rStackHeight, const float rBaseRadius, const float rTipRadius ) :
	m_bInitialized	( false ),
	m_nStacks		( nStacks ),
	m_nSlices		( nSlices ),
	m_rStackHeight	( rStackHeight ),
	m_rBaseRadius	( rBaseRadius ),
	m_rTipRadius	( rTipRadius ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 ),
	m_nCellCount	( 0 ),
	m_nTriCount		( 0 )
{
	m_bInitialized = Initialize();
}
Cylinder::Cylinder( const Cylinder & r ) :
	m_bInitialized	( false ),
	m_nStacks		( r.m_nStacks ),
	m_nSlices		( r.m_nSlices ),
	m_rStackHeight	( r.m_rStackHeight ),
	m_rBaseRadius	( r.m_rBaseRadius ),
	m_rTipRadius	( r.m_rTipRadius ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 ),
	m_nCellCount	( 0 ),
	m_nTriCount		( 0 )
{
	m_bInitialized = Initialize();
}
Cylinder & Cylinder::operator=( const Cylinder & r )
{
	if( &r != this )
	{	
		if( m_bInitialized )
			Uninitialize();

		m_nStacks		= r.m_nStacks;
		m_nSlices		= r.m_nSlices;		
		m_rStackHeight	= r.m_rStackHeight;
		m_rBaseRadius	= r.m_rBaseRadius;
		m_rTipRadius	= r.m_rTipRadius;
				
		m_bInitialized = Initialize();
	}
	
	return *this;
}
Cylinder::~Cylinder()
{
	Uninitialize();
}
	
bool Cylinder::Initialize()
{		
	using AntiMatter::AppLog;

	if( m_bInitialized )
		return false;
	
	if( m_nSlices < 3 || m_nStacks < 2 )
		return false;
	
	if( ! GenerateData() )
	{
		AppLog::Ref().LogMsg("%s data generation failed", __FUNCTION__ );
		return false;
	}

	return true;
}
void Cylinder::Uninitialize()
{
	m_bInitialized = false;

	m_nStacks		= 0;
	m_nSlices		= 0;
	m_rStackHeight	= 0;
	m_rBaseRadius	= 0;
	m_rTipRadius	= 0;

	m_nVertCount	= 0;
	m_nIndexCount	= 0;
	m_nCellCount	= 0;
	m_nTriCount		= 0;

	_pNastyHackPtr	= NULL;
	
	_vertices.clear();
	_indices.clear();
}
	
// -- IGeometry
bool Cylinder::GenerateData()
{
	using AntiMatter::g_Pi;
	using glm::vec3;
	using glm::normalize;
	
	if( m_bInitialized )
		return m_bInitialized;
	
	// -- compute some member variables	
	UINT nRingVerts		= m_nSlices;
	UINT nBaseVerts		= ( nRingVerts + 1 ) * 2;
	UINT nBaseIndices	= ( m_nSlices * 3 ) * 2;

	m_nVertCount		= (m_nSlices * (m_nStacks + 1)) + nBaseVerts;	
	m_nIndexCount		= (m_nStacks * (nRingVerts+1) * 2) + nBaseIndices;
	m_nCellCount		= (m_nSlices) * (m_nStacks-1);
	m_nTriCount			= m_nCellCount * 2;
	
	// -- allocate buffer space for geometry data
	_vertices.reserve( m_nVertCount );
	_indices.reserve( m_nIndexCount );

	// -- generate data
	CreateRingVertices();	
	CreateStackIndices();

	//CreateBaseVertices();
	//CreateTipVertices();

	return true;
}

void Cylinder::CreateRingVertices()
{
	// See Frank D Luna, 3D Programming with Dx11 book, p247-51

	using AntiMatter::g_Pi;
	using glm::vec3;
	using glm::normalize;

	float rHeight		= m_rStackHeight * m_nStacks;
	// float rYStart	= -0.5f * rHeight;
	float rSliceStep	= (1.0f / m_nSlices) * 2.0f * g_Pi;
	float rStackStep	= (1.0f / m_nStacks) * rHeight;	
	float rRadiusStep	= (1.0f / m_nStacks) * (m_rTipRadius - m_rBaseRadius);
	float dr			= m_rBaseRadius - m_rTipRadius;
	int   nRings		= m_nStacks + 1;
	float rRadius;
	float y;
	float s, c;

	for( int i = 0; i < nRings; ++ i )
	{
		y		= (i * m_rStackHeight);
		rRadius = m_rBaseRadius + (i * rRadiusStep);

		for( int j = 0; j <= m_nSlices; j ++ )
		{
			s = sinf( j * rSliceStep );
			c = cosf( j * rSliceStep );

			CustomVertex cv;

			cv.m_Position[X]	= rRadius * c;
			cv.m_Position[Y]	= y;
			cv.m_Position[Z]	= rRadius * s;

			cv.m_TexCoords[U]	= (float) j / m_nSlices;
			cv.m_TexCoords[V]	= 1.0f - (float) i / m_nStacks;
			
			cv.m_Tangent[X]		= -s;
			cv.m_Tangent[Y]		= 0.0f;
			cv.m_Tangent[Z]		= c;
															
			cv.m_Bitangent[X]	= dr * c;
			cv.m_Bitangent[Y]	= -rHeight;
			cv.m_Bitangent[Z]	= dr * s;
						
			vec3 vT(cv.m_Tangent[X], cv.m_Tangent[Y], cv.m_Tangent[Z]);
			vec3 vB(cv.m_Bitangent[X], cv.m_Bitangent[Y], cv.m_Bitangent[Z]);
			vec3 vN = glm::normalize( glm::cross(vT, vB) );
						
			cv.m_Normal[X]		= vN.x;
			cv.m_Normal[Y]		= vN.y;
			cv.m_Normal[Z]		= vN.z;

			memset( cv.m_Colour, 0, sizeof(float)*4 );

			_vertices.push_back(cv);
		}
	}

	
}
void Cylinder::CreateStackIndices()
{
	// indices for each stack
	// Note: The indices are created such that the object is expected
	// to be drawn as follows:
	// 1 triangle strip per stack
	// 1 triangle (fan? strip?) per base/tip
	// e.g. you should not attempt to draw this with just one draw call.


	unsigned short nRingVerts	= m_nSlices + 1;
	unsigned short n			= 0;
	
	for( unsigned short i = 0; i < m_nStacks; ++ i )
	{		
		for( unsigned short j = 0; j < nRingVerts; ++ j )
		{
			_indices.push_back( n ++ );
			_indices.push_back( (unsigned short)m_nSlices + n );			
		}
	}
	

	/*
	unsigned short nRingVerts = m_nSlices + 1;
	unsigned short a,b,c,d,e,f;

	for( unsigned short i = 0; i < m_nStacks; ++ i )
	{		
		for( unsigned short j = 0; j < m_nSlices; ++ j )
		{
			a = (i+1) * nRingVerts + j;
			b = a-5;						// i * nRingVerts + j;
			c = a+1;
			d = c;
			e = b;
			f = e+1;

			_indices.push_back(a);
			_indices.push_back(b);
			_indices.push_back(c);
			_indices.push_back(d);
			_indices.push_back(e);
			_indices.push_back(f);
		}
	}
	*/

	/*
	for( unsigned short i = 0; i < m_nStacks; ++ i )
	{		
		for( unsigned short j = 0; j < m_nSlices; ++ j )
		{
			// 1
			_indices.push_back( (i+1) * m_nSlices + j );

			// 2
			_indices.push_back( i * m_nSlices + j );

			// 3 & 4
			if( j != m_nSlices )
			{
				_indices.push_back( (i+1) * m_nSlices + j + 1 );
				_indices.push_back( (i+1) * m_nSlices + j + 1 );
			}
			else
			{
				_indices.push_back( (i+1) * m_nSlices );
				_indices.push_back( (i+1) * m_nSlices );				
			}

			// 5
			_indices.push_back( i * m_nSlices + j );

			// 6
			if( j != m_nSlices )
				_indices.push_back( i * m_nSlices + j + 1);
			else
				_indices.push_back( i * m_nSlices );
		}				
	}
	*/
	
	/*
	
	unsigned short nRingVerts = m_nSlices + 1;

	for( unsigned short i = 0; i < m_nStacks; ++ i )
	{
		for( unsigned short j = 0; j < m_nSlices; ++ j )
		{
			_indices.push_back( i * nRingVerts + j );
			_indices.push_back( (i+1) * nRingVerts + j );
			_indices.push_back( (i+1) * nRingVerts + j + 1);

			_indices.push_back( i * nRingVerts + j );
			_indices.push_back( (i+1) * nRingVerts + j + 1);
			_indices.push_back( i * nRingVerts + j + 1);
		}
	}
	*/
}

void Cylinder::CreateBaseVertices()
{
	using AntiMatter::g_Pi;
	using glm::vec3;
	using glm::normalize;
	
	float rHeight		= m_nStacks * m_rStackHeight;
	float y				= 0;
	const float	rTheta	= 2.0f * g_Pi / m_nSlices;
	UINT nBaseIndex		= _vertices.size();

	// duplicate the base ring vertices, because the tex coords and normals differ
	for( UINT i = 0; i < (UINT)m_nSlices; ++ i )
	{
		CustomVertex cv;

		// cv.x = m_rBaseRadius * cosf( i * rTheta );
		// cv.y = y;
		// cv.z = m_rBaseRadius * sinf( i * rTheta );
		cv.m_Position[X] = m_rBaseRadius * cosf( i * rTheta );
		cv.m_Position[Y] = y;
		cv.m_Position[Z] = m_rBaseRadius * sinf( i * rTheta);
		
		// cv.u = cv.x / rHeight;
		// cv.v = cv.z / rHeight;
		cv.m_TexCoords[U] = cv.m_Position[X] / rHeight;
		cv.m_TexCoords[V] = cv.m_Position[Z] / rHeight;
		
		// cv.nx = 0.0f;
		// cv.ny = -1.0f;
		// cv.nz = 0.0f;
		cv.m_Normal[X] = 0.0f;
		cv.m_Normal[Y] = -1.0f;
		cv.m_Normal[Z] = 0.0f;
		

		// cv.bitx = 1.0f;
		// cv.bity = 0.0f;
		// cv.bitz = 0.0f;
		cv.m_Bitangent[0] = 1.0f;
		cv.m_Bitangent[1] = 0.0f;
		cv.m_Bitangent[2] = 0.0f;

		// cv.tx = 0.0f;
		// cv.ty = 0.0f;
		// cv.tz = 1.0f;
		cv.m_Tangent[0] = 0.0f;
		cv.m_Tangent[1] = 0.0f;
		cv.m_Tangent[2] = 1.0f;

		_vertices.push_back( cv );
	}

	// Add a center vertex to the cylinder base cap
	UINT nCenterIndex = _vertices.size();
	CustomVertex vtxCenter;

	vtxCenter.m_Position[X] = 0.0f;
	vtxCenter.m_Position[Y] = 0.0f;
	vtxCenter.m_Position[Z] = 0.0f;		

	vtxCenter.m_TexCoords[U] = vtxCenter.m_Position[X] / rHeight;
	vtxCenter.m_TexCoords[V] = vtxCenter.m_Position[Z] / rHeight;		

	vtxCenter.m_Normal[X] = 0.0f;
	vtxCenter.m_Normal[Y] = -1.0f;
	vtxCenter.m_Normal[Z] = 0.0f;		

	vtxCenter.m_Bitangent[0] = 1.0f;
	vtxCenter.m_Bitangent[1] = 0.0f;
	vtxCenter.m_Bitangent[2] = 0.0f;		

	vtxCenter.m_Tangent[0] = 0.0f;
	vtxCenter.m_Tangent[1] = 0.0f;
	vtxCenter.m_Tangent[2] = 1.0f;

	// vtxCenter.x = 0;
	// vtxCenter.y = 0;
	// vtxCenter.z = 0;
	// 
	// vtxCenter.u = vtxCenter.x / rHeight;
	// vtxCenter.v = vtxCenter.z / rHeight;
	// 
	// vtxCenter.nx = 0.0f;
	// vtxCenter.ny = -1.0f;
	// vtxCenter.nz = 0.0f;
	// 
	// vtxCenter.bitx = 1.0f;
	// vtxCenter.bity = 0.0f;
	// vtxCenter.bitz = 0.0f;
	// 
	// vtxCenter.tx = 0.0f;
	// vtxCenter.ty = 0.0f;
	// vtxCenter.tz = 1.0f;
	
	_vertices.push_back( vtxCenter );

	CreateBaseIndices( nBaseIndex, nCenterIndex );

}
void Cylinder::CreateBaseIndices( const UINT nBaseIndex, const UINT nCenterIndex )
{
	for( UINT n = 0; n < (UINT)m_nSlices; ++ n )
	{
		_indices.push_back( (unsigned short)nCenterIndex );
		_indices.push_back( (unsigned short)nBaseIndex + (unsigned short)n + 1 );
		_indices.push_back( (unsigned short)nBaseIndex + (unsigned short)n );
	}
}

void Cylinder::CreateTipVertices()
{
	using AntiMatter::g_Pi;
	using glm::vec3;
	using glm::normalize;

	UINT nBaseIndex		= _vertices.size();
	float rHeight		= m_nStacks * m_rStackHeight;
	float y				= rHeight;
	const float	rTheta	= 2.0f * g_Pi / m_nSlices;

	// duplicate the tip ring vertices, because the tex coords and normals differ
	for( UINT i = 0; i < (UINT)m_nSlices; ++ i )
	{
		CustomVertex cv;

		cv.m_Position[X] = m_rTipRadius * cosf( i * rTheta );
		cv.m_Position[Y] = y;
		cv.m_Position[Z] = m_rTipRadius * sinf( i * rTheta );
		cv.m_TexCoords[U] = cv.m_Position[X] / rHeight;
		cv.m_TexCoords[V] = cv.m_Position[Z] / rHeight;
		cv.m_Normal[X] = 0.0f;
		cv.m_Normal[Y] = 1.0f;
		cv.m_Normal[Z] = 0.0f;		
		cv.m_Bitangent[0] = 1.0f;
		cv.m_Bitangent[1] = 0.0f;
		cv.m_Bitangent[2] = 0.0f;		
		cv.m_Tangent[0] = 0.0f;
		cv.m_Tangent[1] = 0.0f;
		cv.m_Tangent[2] = 1.0f;

		// cv.x = m_rTipRadius * cosf( i * rTheta );
		// cv.y = y;
		// cv.z = m_rTipRadius * sinf( i * rTheta );
		// 
		// cv.u = cv.x / rHeight;
		// cv.v = cv.z / rHeight;
		// 
		// cv.nx = 0.0f;
		// cv.ny = 1.0f;
		// cv.nz = 0.0f;
		// 
		// cv.bitx = 1.0f;
		// cv.bity = 0.0f;
		// cv.bitz = 0.0f;
		// 
		// cv.tx = 0.0f;
		// cv.ty = 0.0f;
		// cv.tz = 1.0f;

		_vertices.push_back( cv );
	}

	// Add a center vertex to the cylinder tip cap
	UINT nCenterIndex = _vertices.size();
	CustomVertex vtxCenter;

	vtxCenter.m_Position[X]		= 0.0f;
	vtxCenter.m_Position[Y]		= (float)rHeight;
	vtxCenter.m_Position[Z]		= 0.0f;
	vtxCenter.m_TexCoords[U]	= vtxCenter.m_Position[X] / rHeight;
	vtxCenter.m_TexCoords[V]	= vtxCenter.m_Position[Z] / rHeight;
	vtxCenter.m_Normal[X]		= 0.0f;
	vtxCenter.m_Normal[Y]		= 1.0f;
	vtxCenter.m_Normal[Z]		= 0.0f;
	vtxCenter.m_Bitangent[0]	= 1.0f;
	vtxCenter.m_Bitangent[1]	= 0.0f;
	vtxCenter.m_Bitangent[2]	= 0.0f;
	vtxCenter.m_Tangent[0]		= 0.0f;
	vtxCenter.m_Tangent[1]		= 0.0f;
	vtxCenter.m_Tangent[2]		= 1.0f;

	// vtxCenter.x = 0;
	// vtxCenter.y = (float) rHeight;
	// vtxCenter.z = 0;
	// 
	// vtxCenter.u = vtxCenter.x / rHeight;
	// vtxCenter.v = vtxCenter.z / rHeight;
	// 
	// vtxCenter.nx = 0.0f;
	// vtxCenter.ny = 1.0f;
	// vtxCenter.nz = 0.0f;
	// 
	// vtxCenter.bitx = 1.0f;
	// vtxCenter.bity = 0.0f;
	// vtxCenter.bitz = 0.0f;
	// 
	// vtxCenter.tx = 0.0f;
	// vtxCenter.ty = 0.0f;
	// vtxCenter.tz = 1.0f;
	
	_vertices.push_back( vtxCenter );

	CreateTipIndices( nBaseIndex, nCenterIndex );

}
void Cylinder::CreateTipIndices( const UINT nTipIndex, const UINT nCenterIndex )
{
	for( UINT n = 0; n < (UINT)m_nSlices; ++ n )
	{
		_indices.push_back( (unsigned short)nCenterIndex );
		_indices.push_back( (unsigned short)nTipIndex + (unsigned short)n + 1 );
		_indices.push_back( (unsigned short)nTipIndex + (unsigned short)n );
	}
}
