
#include "stdafx.h"
#include "Plane.h"
#include "VertexClass.h"
#include <AntiMatter\AppLog.h>
#include <AntiMatter\AppException.h>
#include <vector>
#include <math.h>
#include <comdef.h>

Plane::Plane() :
	m_pVertices		( 0 ),
	m_pdwIndices	( 0 ),
	m_nVertCols		( 0 ),
	m_nVertRows		( 0 ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 ),
	m_nCellRows		( 0 ),
	m_nCellCols		( 0 ),
	m_rWidth		( 0.0f ),
	m_rHeight		( 0.0f ),
	m_rCellWidth	( 0.0f ),
	m_rCellHeight	( 0.0f ),
	m_rCellDepth	( 0.0f ),
	m_nCellCount	( 0 ),
	m_nTriCount		( 0 ),
	m_bInitialized	( false )
{
}
Plane::Plane( int nVertCols, int nVertRows, const float rCellWidth, const float rCellHeight, const float rCellDepth) :
	m_pVertices		( 0 ),
	m_pdwIndices	( 0 ),
	m_nVertCols		( nVertCols ),
	m_nVertRows		( nVertRows ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 ),
	m_nCellRows		( 0 ),
	m_nCellCols		( 0 ),
	m_rWidth		( 0.0f ),
	m_rHeight		( 0.0f ),
	m_rCellWidth	( rCellWidth ),
	m_rCellHeight	( rCellHeight ),
	m_rCellDepth	( rCellDepth ),
	m_nCellCount	( 0 ),
	m_nTriCount		( 0 ),
	m_bInitialized	( false )
{
	Initialize();
}
Plane::Plane( const Plane & rhs )
{
	this->Uninitialize();

	this->m_nVertCols		= rhs.m_nVertCols;
	this->m_nVertRows		= rhs.m_nVertRows;
	this->m_nVertCount		= rhs.m_nVertCount;
	this->m_nIndexCount		= rhs.m_nIndexCount;

	this->m_nCellRows		= rhs.m_nCellRows;
	this->m_nCellCols		= rhs.m_nCellCols;
	this->m_rWidth			= rhs.m_rWidth;
	this->m_rHeight			= rhs.m_rHeight;

	this->m_rCellWidth		= rhs.m_rCellWidth;
	this->m_rCellHeight		= rhs.m_rCellHeight;
	this->m_rCellDepth		= rhs.m_rCellDepth;	
	this->m_nCellCount		= rhs.m_nCellCount;
	this->m_nTriCount		= rhs.m_nTriCount;

	if( rhs.Initialized() )
		this->Initialize();
}
Plane & Plane::operator=( const Plane & rhs )
{
	if( &rhs != this )
	{
		if( this->m_bInitialized )
			this->Uninitialize();

		this->m_nVertCols		= rhs.m_nVertCols;
		this->m_nVertRows		= rhs.m_nVertRows;
		this->m_nVertCount		= rhs.m_nVertCount;
		this->m_nIndexCount		= rhs.m_nIndexCount;

		this->m_nCellRows		= rhs.m_nCellRows;
		this->m_nCellCols		= rhs.m_nCellCols;
		this->m_rWidth			= rhs.m_rWidth;
		this->m_rHeight			= rhs.m_rHeight;

		this->m_rCellWidth		= rhs.m_rCellWidth;
		this->m_rCellHeight		= rhs.m_rCellHeight;
		this->m_rCellDepth		= rhs.m_rCellDepth;	
		this->m_nCellCount		= rhs.m_nCellCount;
		this->m_nTriCount		= rhs.m_nTriCount;

		if( rhs.Initialized() )
			this->Initialize();
	}

	return *this;
}
Plane::~Plane()
{
	try
	{
		Uninitialize();
	}
	catch(...){}
}

bool Plane::Initialize()
{
	HRESULT hr;	

	hr = CreatePlane(	
		m_nVertRows, m_nVertCols, m_rCellWidth, m_rCellHeight, 
		&m_pVertices, &m_pdwIndices
	);	



	m_bInitialized = SUCCEEDED(hr) ? true:false;

	if( ! m_bInitialized )
		Uninitialize();

	return m_bInitialized;
}
void Plane::Uninitialize()
{	
	if( (m_pVertices) )
	{
		delete [] m_pVertices;
		m_pVertices = NULL;
	}

	if( m_pdwIndices )
	{
		delete [] m_pdwIndices;
		m_pdwIndices = NULL;
	}

	m_FaceNormals.clear();

	this->m_nVertCols		= 0;
	this->m_nVertRows		= 0;
	this->m_nVertCount		= 0;
	this->m_nIndexCount		= 0;
	
	this->m_rCellWidth		= 0.0f;
	this->m_rCellHeight		= 0.0f;
	this->m_rCellDepth		= 0.0f;
	this->m_nCellCount		= 0;
	this->m_nTriCount		= 0;

	m_bInitialized = false;
}

HRESULT Plane::CreatePlane( 
	const int			nVertRows, 
	const int			nVertCols, 
	const float			rCellWidth, 
	const float			rCellHeight, 
	CustomVertex**		ppVertices, 
	unsigned short**	ppdwIndices
)
{
	if( (nVertRows <= 0) || (nVertCols <= 0) || (rCellWidth <= 0.0f) || (rCellHeight <= 0.0f) ||
		( ppVertices == NULL ) || (ppdwIndices == NULL ) )
	{
		return E_INVALIDARG;
	}

	using AntiMatter::AppLog;
	
	HRESULT hr			= S_OK;
		
	m_nCellRows			= m_nVertRows - 1;
	m_nCellCols			= m_nVertCols - 1;
	m_rWidth			= (float)m_nCellCols * m_rCellWidth;
	m_rHeight			= (float)m_nCellRows * m_rCellHeight;	
	
	m_nCellCount		= m_nCellRows * m_nCellCols;
	m_nTriCount			= m_nCellRows * m_nCellCols * 2;
	m_nVertCount		= m_nVertRows * m_nVertCols;
	m_nIndexCount		= m_nCellCount * 6;
	

	// generate data
	hr = GenerateVertices( &m_pVertices );
	if( FAILED(hr) )
	{
		AppLog::Ref().LogMsg( 
			"Plane::CreatePlane() GenerateVertices for Plane geometry failed: %s", 
			_com_error(hr).ErrorMessage()
		);
		ATLASSERT(0);
		return hr;
	}

	hr = GenerateIndices( &m_pdwIndices );
	if( FAILED(hr) )
	{
		AppLog::Ref().LogMsg( 
			"Plane::CreatePlane() GenerateIndices for Plane geometry failed: %s", 
			_com_error(hr).ErrorMessage()
		);
		ATLASSERT(0);
		return hr;
	}

	hr = GenerateNormals( &m_pVertices );
	if( FAILED(hr) )
	{
		AppLog::Ref().LogMsg( 
			"Plane::CreatePlane() GenerateNormals for Plane geometry failed: %s", 
			_com_error(hr).ErrorMessage()
		);
		ATLASSERT(0);
		return hr;
	}
	
	return hr;
}

HRESULT Plane::GenerateVertices( CustomVertex** ppVertices )
{
	// CreateVertices and tex coords
	// see the Quad "IGeometry" class for a simpler implementation of this
	// algorithm

	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppVertices )
		return E_INVALIDARG;

	*ppVertices	= new (std::nothrow) CustomVertex[m_nVertCount];
	if( ! *ppVertices )
		return E_OUTOFMEMORY;		

	int n				= 0;
	float rHalfWidth	= m_rWidth / 2.0f;
	float rHalfHeight	= m_rHeight / 2.0f;

	for( int h = 0; h < m_nVertRows; h ++ )
	{
		for( int w = 0; w < m_nVertCols; w ++ )
		{							
			(*ppVertices)[n].m_Position[X]	= (float)(( w * m_rCellWidth ) - rHalfWidth );
			(*ppVertices)[n].m_Position[Y]	= 0.0f;
			(*ppVertices)[n].m_Position[Z]	= (float)(( h * m_rCellHeight ) - rHalfHeight );

			(*ppVertices)[n].m_TexCoords[U] = ((float)w / (m_nVertCols-1.0f) );
			(*ppVertices)[n].m_TexCoords[V] = ((float)h / (m_nVertRows-1.0f) );

			n ++;
		}
	}

	return S_OK;
}
HRESULT Plane::GenerateIndices( unsigned short** ppdwIndices )
{	
	// iterate the quads and make indices for two triangles per quad
	// see the Quad "IGeometry" class for a simpler implementation of this
	// algorithm

	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppdwIndices )
		return E_INVALIDARG;

	*ppdwIndices = new (std::nothrow) unsigned short[m_nIndexCount];
	if ( ! *ppdwIndices )	
		return E_OUTOFMEMORY;

	int n = 0;
	
	/*
	// (clockwise) winding order
	for( int h = 0; h < nCellRows; h ++ )
	{
		for( int w = 0; w < nCellCols; w ++ )
		{
			(*ppdwIndices)[n]	= h		* nVertCols + w;
			(*ppdwIndices)[n+1] = h		* nVertCols + w + 1;
			(*ppdwIndices)[n+2] = (h+1)	* nVertCols + w;

			(*ppdwIndices)[n+3] = (h+1)	* nVertCols + w;
			(*ppdwIndices)[n+4] = h		* nVertCols + w + 1;
			(*ppdwIndices)[n+5] = (h+1) * nVertCols + w + 1;

			n += 6;
		}
	}
	*/
	
	// (anticlockwise) winding order
	for( int h = 0; h < m_nCellRows; h ++ )
	{
		for( int w = 0; w < m_nCellCols; w ++ )
		{
			(*ppdwIndices)[n]	= (unsigned short)(h		* m_nVertCols + w);
			(*ppdwIndices)[n+1] = (unsigned short)((h+1)	* m_nVertCols + w);
			(*ppdwIndices)[n+2] = (unsigned short)((h)	* m_nVertCols + w + 1);

			(*ppdwIndices)[n+3] = (unsigned short)((h+1)	* m_nVertCols + w);
			(*ppdwIndices)[n+4] = (unsigned short)((h+1)	* m_nVertCols + w + 1);
			(*ppdwIndices)[n+5] = (unsigned short)(h		* m_nVertCols + w + 1);

			n += 6;
		}
	}

	return S_OK;
}
HRESULT Plane::GenerateNormals( CustomVertex** ppVertices )
{
	// Normals
	// for a flat x-z plane, each normal is a unit vector in the y-direction

	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! *ppVertices )
		return E_INVALIDARG;

	int n = 0;
	
	for( int h = 0; h < m_nVertRows; h ++ )
	{
		for( int w = 0; w < m_nVertCols; w ++ )
		{			
			(*ppVertices)[n].m_Normal[X] = 0.0f;
			(*ppVertices)[n].m_Normal[Y] = 1.0f;
			(*ppVertices)[n].m_Normal[Z] = 0.0f;

			n ++;
		}
	}

	return S_OK;
}

void Plane::RecalcNormalMap()
{
	using namespace std;
	using namespace AntiMatter;

	vector<int> vnFaces;

	ComputeFaceNormals();

	// Iterate the vertices and compute normal for each
	for( int v = 0; v < m_nVertCount; v ++ )
	{
		glm::vec3 vNormal;

		vnFaces.clear();

		IdentifyFacesForVertex( vnFaces, v );
		ComputeVNormalFromFaceList( vnFaces, vNormal );

		(m_pVertices)[v].m_Normal[0] = vNormal.x;
		(m_pVertices)[v].m_Normal[1] = vNormal.y;
		(m_pVertices)[v].m_Normal[2] = vNormal.z;
	}
}
void Plane::ComputeFaceNormals()
{
	using namespace glm;
	using namespace std;

	m_FaceNormals.clear();

	if( m_nTriCount <= 0 || m_nVertCount <= 0 )	
		return;	

	m_FaceNormals.resize(m_nTriCount);
	
	for( int nFace = 0; nFace < m_nTriCount; nFace ++ )
	{
		int nFaceIdx	= nFace * 2;
		int n1			= m_pdwIndices[nFaceIdx];
		int n2			= m_pdwIndices[nFaceIdx+1];
		int n3			= m_pdwIndices[nFaceIdx+2];

		//CustomVertex*	pv1 = &m_pVertices[n1];
		//CustomVertex*	pv2 = &m_pVertices[n2];
		//CustomVertex*	pv3 = &m_pVertices[n3];

		float* pV1 = (&m_pVertices[n1].m_Position[0]);
		float* pV2 = (&m_pVertices[n2].m_Position[0]);
		float* pV3 = (&m_pVertices[n3].m_Position[0]);
		
		vec3 v1(pV1[0], pV1[1], pV1[2]);
		vec3 v2(pV2[0], pV2[1], pV2[2]);
		vec3 v3(pV3[0], pV3[1], pV3[2]);
		
		m_FaceNormals.push_back( normalize( cross(v3-v1, v2-v1) ) );
		

		//vec3 v1			= vec3(p2) - vec3(pv1->m_Position);
		//vec3 v2			= vec3(pv3->m_Position) - vec3(pv1->m_Position);

		// normalize the cross product of v1 & v2 and store
		m_FaceNormals.push_back( glm::normalize(v1*v2) );
		// m_FaceNormals.push_back( AntiMatter::Vec3::Normalize(v1*v2) );
	}
}
void Plane::IdentifyFacesForVertex( std::vector<int> & vnFaces, int nVertIndex )
{
	// Determine the vertex row & vertex column the vertex is in
	// Populate a list of the faces the vertex is in
	// first identify the vertex rows&cols, from that determine the cell rows&cols
	// from there identify the faces that use the vertex
	//
	// cells affected are 
	// (vRow-1, vCol-1)
	// (vRow-1, vCol)
	// (vRow,	vCol-1)
	// (vRow,	vCol )

	int nVertRow	= abs( nVertIndex / m_nVertCols );
	int nVertCol	= nVertIndex - (m_nVertCols*nVertRow);	
	int nCellRow	= nVertRow - 1;
	int nCellCol	= nVertCol - 1;
	int nOffset		= (nCellCol*2) + (nCellRow*2);

	if( (nCellRow >= 0) && (nCellCol >= 0) )
	{
		// (0,3)
		// vnFaces.push_back( nOffset );
		vnFaces.push_back( nOffset+1 );
	}
	

	// (0,4)
	nCellCol ++;
	if( (nCellRow >= 0) && (nCellCol >= 0) )
	{
		
		nOffset	= (nCellCol*2) + (nCellRow*2);
		
		vnFaces.push_back( nOffset );
		vnFaces.push_back( nOffset+1 );
	}
	
	// (1, 3)
	nCellCol --;
	nCellRow ++;
	if( (nCellRow >= 0) && (nCellCol >= 0) )
	{		
		nOffset	= (nCellCol*2) + (nCellRow*2);

		vnFaces.push_back( nOffset );
		vnFaces.push_back( nOffset+1 );
	}
	

	// (1, 4)
	nCellRow ++;
	if( (nCellRow >= 0) && (nCellCol >= 0) )
	{
		nOffset	= (nCellCol*2) + (nCellRow*2);

		vnFaces.push_back( nOffset );
		// vnFaces.push_back( nOffset+1 );
	}

	return;

}
void Plane::ComputeVNormalFromFaceList( std::vector<int> & vnFaces, glm::vec3 & vNormal )
{
	// Given the face list relating to a specified vertex, this function iterates the 
	// faces in the face list and: 
	// sums the normals
	// normalises the resultant vNormal

	typedef std::vector<int>::iterator Vitr;	

	int nIndex;

	for( Vitr x = vnFaces.begin(); x != vnFaces.end(); x ++ )
	{		
		nIndex = *x;		
		vNormal += m_FaceNormals[nIndex];
	}

	glm::normalize( vNormal );	
}
