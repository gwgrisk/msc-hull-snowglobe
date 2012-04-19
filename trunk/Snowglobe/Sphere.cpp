
#pragma once
#include "stdafx.h"
#include "Sphere.h"

#include "IGraphNode.h"
#include "IGeometry.h"
#include "SceneGraph.h"
#include "VertexClass.h"

#include <string>
#include <glm\glm.hpp>
#include <AntiMatter\AppLog.h>
#include <AntiMatter\constants.h>
#include <comdef.h>

#include "VertexClass.h"

Sphere::Sphere() :
	m_bInitialized	( false ),
	m_nStacks		( 0 ),
	m_nSlices		( 0 ),
	m_rRadius		( 0.0f ),
	m_nVertCount	( 0 ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL )
{	
}
Sphere::Sphere( const float rRadius, const int nSlices, const int nStacks ) :
	m_bInitialized	( false ),
	m_nStacks		( nStacks ),
	m_nSlices		( nSlices ),
	m_rRadius		( rRadius ),
	m_nVertCount	( 0 ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL )
{
	Initialize();
}
Sphere::Sphere( const Sphere & rhs ) :
	m_bInitialized	( rhs.m_bInitialized ),
	m_nStacks		( rhs.m_nStacks ),
	m_nSlices		( rhs.m_nSlices ),
	m_rRadius		( rhs.m_rRadius ),
	m_nVertCount	( rhs.m_nVertCount ),
	m_pVertices		( NULL ),	
	m_pdwIndices	( NULL )
{
	Uninitialize();
	Initialize();
}
Sphere & Sphere::operator=( const Sphere & rhs )
{
	if( this != &rhs )
	{
		Uninitialize();
		
		m_nStacks		= rhs.m_nStacks;
		m_nSlices		= rhs.m_nSlices;
		m_rRadius		= rhs.m_rRadius;
		m_nVertCount	= rhs.m_nVertCount;
		m_bInitialized	= false;
		m_pVertices		= NULL;
		m_pdwIndices	= NULL;
		
		Initialize();
	}

	return *this;
}
Sphere::~Sphere()
{
	try
	{
		Uninitialize();
	}
	catch(...)
	{
	}
}

bool Sphere::Initialize()
{
	if( m_bInitialized )
		return true;

	if( (m_nStacks < 2) && (m_nSlices < 2) )
		return false;

	using AntiMatter::AppLog;

	m_nVertCount	= (m_nSlices + 1)* (m_nStacks-1) + 2;	// + 2 is for the top and bottom vertex;
	m_nIndexCount	= 2 + (m_nStacks-1) * (m_nSlices+1) * 2;
		
	HRESULT hr;

	hr = GenerateVertices( &m_pVertices );
	if( ! SUCCEEDED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Sphere::Initialize() GenerateVertices failure: %s",
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	hr = GenerateIndices(&m_pdwIndices);
	if( ! SUCCEEDED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Sphere::Initialize() GenerateIndices failure: %s",
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	m_bInitialized = true;

	return m_bInitialized;
}
void Sphere::Uninitialize()
{
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

	m_nStacks		= 0;
	m_nSlices		= 0;
	m_rRadius		= 0.0f;
	m_nVertCount	= 0;
	m_nIndexCount	= 0;
	m_bInitialized	= false;
}

// IGeometry
HRESULT Sphere::GenerateVertices( CustomVertex** ppVertices )
{
	using AntiMatter::g_Pi;

	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppVertices )
		return E_INVALIDARG;

	*ppVertices = new (std::nothrow) CustomVertex[m_nVertCount];
	if( ! *ppVertices )
		return E_OUTOFMEMORY;
	
	int n = 0;

	// top point of sphere
	(*ppVertices)[n].m_Position[X]	= 0.0f;
	(*ppVertices)[n].m_Position[Y]	= m_rRadius;
	(*ppVertices)[n].m_Position[Z]	= 0.0f;

	(*ppVertices)[n].m_Normal[X]	= 0.0f;
	(*ppVertices)[n].m_Normal[Y]	= 1.0f;
	(*ppVertices)[n].m_Normal[Z]	= 0.0f;

	(*ppVertices[n]).m_TexCoords[U]	= 0.0f;
	(*ppVertices[n]).m_TexCoords[V] = 1.0f;

	n ++;

	// bottom point of sphere
	(*ppVertices)[n].m_Position[X]	= 0.0f;
	(*ppVertices)[n].m_Position[Y]	= -m_rRadius;
	(*ppVertices)[n].m_Position[Z]	= 0.0f;

	(*ppVertices)[n].m_Normal[X]	= 0.0f;
	(*ppVertices)[n].m_Normal[Y]	= -1.0f;
	(*ppVertices)[n].m_Normal[Z]	= 0.0f;

	(*ppVertices)[n].m_TexCoords[U]	= 0.0f;
	(*ppVertices)[n].m_TexCoords[V] = 0.0f;

	n ++;

	float rStackStep = 1.0f / (float)m_nStacks;
	float rSliceStep = 2.0f * g_Pi / (float)m_nSlices;

	float	x, y, z;
	float	rad;
	int		nTmp;
	float	rTmpTex;


	for( int i = 1; i < m_nStacks; i ++ )
	{
		y		= sin( g_Pi * ( 1/2.0f - rStackStep * (float)i) );
		rad		= cos( g_Pi * ( 1/2.0f - rStackStep * (float)i) );
		nTmp	= n;
		rTmpTex	= 1.0f - rStackStep * (float)i;

		for( int j = 0; j < m_nSlices; j ++ )
		{
			x = cos ( (float)j * rSliceStep );
			z = -sin( (float)j * rSliceStep );

			(*ppVertices)[n].m_Position[0]	= m_rRadius * rad * x;
			(*ppVertices)[n].m_Position[1]	= m_rRadius * y;
			(*ppVertices)[n].m_Position[2]	= m_rRadius * rad * z;

			(*ppVertices)[n].m_Normal[0]	= (rad * x);
			(*ppVertices)[n].m_Normal[1]	= (y);
			(*ppVertices)[n].m_Normal[2]	= (rad * z);

			(*ppVertices)[n].m_TexCoords[0]	= (float)j / (float)m_nSlices;
			(*ppVertices)[n].m_TexCoords[1]	= rTmpTex;

			n ++;
		}

		(*ppVertices)[n].m_Position[0]	= (*ppVertices)[nTmp].m_Position[0];
		(*ppVertices)[n].m_Position[1]	= (*ppVertices)[nTmp].m_Position[1];
		(*ppVertices)[n].m_Position[2]	= (*ppVertices)[nTmp].m_Position[2];

		(*ppVertices)[n].m_Normal[0]	= (*ppVertices)[nTmp].m_Normal[0];
		(*ppVertices)[n].m_Normal[1]	= (*ppVertices)[nTmp].m_Normal[1];
		(*ppVertices)[n].m_Normal[2]	= (*ppVertices)[nTmp].m_Normal[2];

		(*ppVertices)[n].m_TexCoords[0] = 1;
		(*ppVertices)[n].m_TexCoords[1] = rTmpTex;

		n ++;
	}

	return S_OK;
}
HRESULT Sphere::GenerateIndices( DWORD ** ppdwIndices )
{
	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppdwIndices )
		return E_INVALIDARG;

	*ppdwIndices = new (std::nothrow) DWORD[m_nIndexCount];
	if( ! *ppdwIndices )
		return E_OUTOFMEMORY;

	int n	= 0;
	int v	= 2;

	// top
	(*ppdwIndices)[n++] = 0;

	for( int j = 0; j <= m_nSlices; j ++ )
	{
		(*ppdwIndices)[n++] = v ++;
	}
	
	v -= (m_nSlices+1);

	// strips
	for( int i = 0; i < (m_nStacks-2); i ++ )
	{
		for( int j  = 0; j <= m_nSlices; j ++ )
		{ 
			(*ppdwIndices)[n++] = v ++;
			(*ppdwIndices)[n++] = m_nSlices + v;
		}
	}

	// bottom
	(*ppdwIndices)[n++] = 1;

	for( int j = 0; j <= m_nSlices; j ++ )
	{
		(*ppdwIndices)[n++] = v + m_nSlices - j;
	}

	ATLASSERT( n <= m_nIndexCount );

	return S_OK;
}