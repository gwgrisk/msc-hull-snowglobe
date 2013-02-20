
#include "stdafx.h"
#include "Pbo.h"

Pbo::Pbo( GLuint nBindPt, GLuint nPixelDataSize, void* pPixelData, GLuint nBufferUsage ) : 
	m_bInitialized	( false ),
	m_nId			( 0 ),
	m_nBindPt		( nBindPt ),	
	m_nPixelDataSize( nPixelDataSize ),
	m_pPixelData	( pPixelData ),
	m_nBufferUsage	( nBufferUsage )
{
	Initialize();
}
const Pbo & Pbo::operator=( const Pbo & r )
{
	if( &r != this )
	{
		if( m_bInitialized )
			Uninitialize();
		
		m_nBindPt			= r.m_nBindPt;
		m_nPixelDataSize	= r.m_nPixelDataSize;
		m_pPixelData		= r.m_pPixelData;
		m_nBufferUsage		= r.m_nBufferUsage;

		Initialize();
	}

	return *this;
}

Pbo::~Pbo()
{
	Uninitialize();
}

bool Pbo::Initialize()
{
	glex::Load();

	glGenBuffers(1, &m_nId );

	if( m_nId == 0 )
		return false;
	
	if( m_nBindPt == GL_PIXEL_PACK_BUFFER )
	{
		glBindBuffer( GL_PIXEL_PACK_BUFFER, m_nId );
		glBufferData( GL_PIXEL_PACK_BUFFER, m_nPixelDataSize, m_pPixelData, m_nBufferUsage );
		glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
	}

	return true;
}
void Pbo::Uninitialize()
{
	if( ! m_bInitialized )
		return;

	m_bInitialized = false;	

	if( m_nId != 0 )	
		glDeleteBuffers( 1, &m_nId );
	
	m_nId				= 0;
	m_nBindPt			= GL_PIXEL_PACK_BUFFER;
	m_nPixelDataSize	= 0;
	m_pPixelData		= NULL;
	m_nBufferUsage		= GL_DYNAMIC_COPY;
}