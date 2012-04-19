
#pragma once
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


// references

Cylinder::Cylinder() :
	m_bInitialized	( false ),
	m_nStacks		( 2 ),
	m_nSlices		( 8 ),
	m_rStackHeight	( 4.0f ),
	m_rBaseRadius	( 10.0f ),
	m_rTipRadius	( 10.0f ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 ),
	m_nCellCount	( 0 ),
	m_nTriCount		( 0 ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL )
{
	//m_bInitialized = Initialize();
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
	m_nTriCount		( 0 ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL )
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
	m_nTriCount		( 0 ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL )
{
	if( r.Initialized() )
	{
		m_bInitialized = Initialize();
	}
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
		
		if( r.Initialized() )
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

	m_nVertCount	= m_nSlices * m_nStacks;	
	m_nIndexCount	= (m_nSlices+1) * m_nStacks * 6;	
	m_nCellCount	= (m_nSlices) * (m_nStacks-1);
	m_nTriCount		= m_nCellCount * 2;
	
	
	HRESULT hr;
	hr = GenerateVertices( &m_pVertices );
	if( FAILED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Cylinder::Initialize() GenerateVertices failed: %s",
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	hr = GenerateIndices( &m_pdwIndices );
	if( FAILED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Cylinder::Initialize() GenerateIndices failed: %s",
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	return true;
}
void Cylinder::Uninitialize()
{
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

	m_nVertCount	= 0;
	m_nIndexCount	= 0;
}
	
// -- IGeometry
HRESULT Cylinder::GenerateVertices( CustomVertex** ppVertices )
{
	// Generates vertices for a cylinder centered around the origin on the Y-axis
	// this was a good idea in principal, but it makes the tree more complex to draw,
	// so as a nasty little hack, I've changed the cylinder so that its 
	// "center of gravity" is centered on its base!

	using AntiMatter::g_Pi;

	if( m_bInitialized )
		return E_UNEXPECTED;
	
	if( ! ppVertices )
		return E_INVALIDARG;		

	*ppVertices = new (std::nothrow) CustomVertex[m_nVertCount];
	if( ! *ppVertices )
		return E_OUTOFMEMORY;
	
	//float	rHalfHeight	= (m_rStackHeight * m_nStacks) / 2.0f;
	float	rRadius		= m_rBaseRadius;
	float	rRadiusStep	= (m_rBaseRadius - m_rTipRadius) / (float) m_nStacks;	
	float	rTheta		= 0.0f;
	int		nVi			= 0;

	float	rX, rY, rZ;

	for( int y = 0; y < m_nStacks; y ++ )
	{
		rRadius = m_rBaseRadius - (rRadiusStep * y);		

		for( int x = 0; x < m_nSlices; x ++ )
		{
			rTheta	= (2 * g_Pi) / m_nSlices;
			rX		= rRadius * sinf( x * rTheta );
			rZ		= rRadius * -cosf( x * rTheta );
			rY		= ( y * m_rStackHeight );

			// vertices
			(*ppVertices)[nVi].m_Position[X]	= rX;
			(*ppVertices)[nVi].m_Position[Y]	= rY;
			(*ppVertices)[nVi].m_Position[Z]	= rZ;			

			// normals
			(*ppVertices)[nVi].m_Normal[X]		= cosf( x*rTheta );
			(*ppVertices)[nVi].m_Normal[Y]		= 0;
			(*ppVertices)[nVi].m_Normal[Z]		= -sinf( x*rTheta );

			// tex-coords
			(*ppVertices)[nVi].m_TexCoords[U]	= rX / sqrt((rX*rX)+(rY*rY)+(rZ*rZ));
			(*ppVertices)[nVi].m_TexCoords[V]	= rY / sqrt((rX*rX)+(rY*rY)+(rZ*rZ));

			nVi ++; // vertex index
		}
	}

	if( nVi != m_nVertCount )
	{
		ATLTRACE("Cylinder::GenerateVertices() Error in your calculations?\r\n");
		ATLASSERT(0);
	}

	return S_OK;
}
HRESULT Cylinder::GenerateIndices( DWORD** ppdwIndices )
{
	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppdwIndices )
		return E_INVALIDARG;

	*ppdwIndices = new (std::nothrow) DWORD[m_nIndexCount];
	if ( ! *ppdwIndices )
		return E_OUTOFMEMORY;
	

	int n = 0;	
	int v = 0;

	for( int y = 0; y < m_nStacks; y ++ )
	{
		for( int x = 0; x < (m_nSlices+1); x ++ )
		{ 
			(*ppdwIndices)[n++] = v ++;
			(*ppdwIndices)[n++] = m_nSlices + v;			
		}
	}


	return S_OK;
}
