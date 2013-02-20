
#include "stdafx.h"
#include "Tri.h"

#include <AntiMatter\AppLog.h>
#include <comdef.h>

Tri::Tri() : 
	m_bInitialized	( false ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL ),
	m_rBaseLength	( 10.0f ),
	m_rHeight		( 5.0f ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 )
{
	m_bInitialized = Initialize();	
}
Tri::Tri( const float rBaseLength, const float rHeight ): 
	m_bInitialized	( false ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL ),
	m_rBaseLength	( rBaseLength ),
	m_rHeight		( rHeight ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 )
{
	m_bInitialized = Initialize();	
}
Tri::Tri( const Tri & r ) : 
	m_bInitialized	( false ),
	m_pVertices		( NULL ),
	m_pdwIndices	( NULL ),
	m_rBaseLength	( r.m_rBaseLength ),
	m_rHeight		( r.m_rHeight ),
	m_nVertCount	( 0 ),
	m_nIndexCount	( 0 )
{
	if( r.Initialized() )
	{
		m_bInitialized = Initialize();
	}			
}
Tri & Tri::operator=( const Tri & r )
{	
	if( this != &r )
	{
		if( m_bInitialized )
			Uninitialize();

		m_rBaseLength	= r.m_rBaseLength;
		m_rHeight		= r.m_rHeight;

		if( r.Initialized() )
			m_bInitialized = Initialize();
	}

	return *this;
}
Tri::~Tri()
{
	Uninitialize();
}

bool Tri::Initialize()
{	
	if( m_bInitialized )
		return true;

	using AntiMatter::AppLog;
	using std::string;

	HRESULT hr		= S_OK;
	m_nVertCount	= 3;
	m_nIndexCount	= 3;

	hr = GenerateVertices( &m_pVertices );
	if( ! SUCCEEDED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Tri::Initialize() failed to GenerateVertices(): %s", 
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	hr = GenerateIndices( &m_pdwIndices );
	if( ! SUCCEEDED(hr) )
	{
		AppLog::Ref().LogMsg(
			"Tri::Initialize() failed to GenerateIndices(): %s", 
			_com_error(hr).ErrorMessage()
		);
		Uninitialize();
		return false;
	}

	m_bInitialized = true;

	return true;
}
void Tri::Uninitialize()
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

	m_rBaseLength	= 0.0f;
	m_rHeight		= 0.0f;
	m_nVertCount	= 0;
	m_nIndexCount	= 0;	
}
	
// IGeometry
HRESULT Tri::GenerateVertices( CustomVertex** ppVertices )
{
	if( m_bInitialized )
		return E_UNEXPECTED;

	if( ! ppVertices )
		return E_INVALIDARG;

	*ppVertices = new (std::nothrow) CustomVertex[m_nVertCount];
	if( ! *ppVertices )
		return E_OUTOFMEMORY;

	// vertices
	(*ppVertices)[1].m_Position[X] = 0.0f;					// tip top
	(*ppVertices)[1].m_Position[Y] = m_rHeight / 2.0f;
	(*ppVertices)[1].m_Position[Z] = 0.0f;

	(*ppVertices)[1].m_Position[X] = -m_rBaseLength / 2.0f;		// lower left corner
	(*ppVertices)[1].m_Position[Y] = 0.0f;
	(*ppVertices)[1].m_Position[Z] = -m_rHeight /2.0f;

	(*ppVertices)[2].m_Position[X] = m_rBaseLength / 2.0f;		// lower right corner
	(*ppVertices)[2].m_Position[Y] = 0.0f;
	(*ppVertices)[2].m_Position[Z] = -m_rHeight /2.0f;	
	

	// texture coords
	(*ppVertices)[0].m_TexCoords[U] = 0.0f;
	(*ppVertices)[0].m_TexCoords[V] = 1.0f;	

	(*ppVertices)[1].m_TexCoords[U] = 1.0f;
	(*ppVertices)[1].m_TexCoords[V] = 1.0f;	

	(*ppVertices)[2].m_TexCoords[U] = 1.0f;
	(*ppVertices)[2].m_TexCoords[V] = 0.0f;

	// normals	
	// per vertex colours ( all set white )
	for( int n = 0; n < m_nVertCount; n ++ )
	{
		(*ppVertices)[n].m_Normal[X] = 0.0f;
		(*ppVertices)[n].m_Normal[Y] = 1.0f;
		(*ppVertices)[n].m_Normal[Z] = 0.0f;

		(*ppVertices)[n].m_Colour[R] = 1.0f;
		(*ppVertices)[n].m_Colour[G] = 1.0f;
		(*ppVertices)[n].m_Colour[B] = 1.0f;
		(*ppVertices)[n].m_Colour[A] = 1.0f;
	}
	
	return S_OK;
}
HRESULT Tri::GenerateIndices( unsigned short** ppIndices )
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

	return S_OK;
}
