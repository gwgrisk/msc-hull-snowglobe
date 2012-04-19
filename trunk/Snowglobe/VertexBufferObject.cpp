
#include "stdafx.h"
#include <GXBase.h>
#include "VertexBufferObject.h"
#include "VertexClass.h"

VertexBufferObject::VertexBufferObject() :	
	m_nVboId		( 0 ),
	m_nVboIdIndex	( 0 ),
	m_bInitialized	( false ),
	m_nNumVertices	( 0 ),
	m_nNumIndices	( 0 ),
	m_ppVertices	( NULL ),
	m_pIndices		( NULL )
{	
}
VertexBufferObject::VertexBufferObject( int nNumVerts, CustomVertex** ppVertices, int nNumIndices, DWORD* pIndices ) :	
	m_nVboId		( 0 ),
	m_nVboIdIndex	( 0 ),
	m_bInitialized	( false ),
	m_nNumVertices	( nNumVerts ),
	m_nNumIndices	( nNumIndices ),
	m_ppVertices	( ppVertices ),
	m_pIndices		( pIndices )
{
	glex::Load();
	Initialize();
}
VertexBufferObject::~VertexBufferObject()
{
	try
	{
		Unload();
	}
	catch(...)
	{
	}
}

void VertexBufferObject::Initialize()
{	
	if( (m_nNumVertices <= 0) || (! *m_ppVertices) )
		return;

	CopyToGfxMem();

	m_bInitialized = true;
	return;
}
bool VertexBufferObject::Initialized() const
{
	return m_bInitialized;	
}
void VertexBufferObject::CopyToGfxMem()
{
	glGenBuffers( 1, &m_nVboId );							// request a VBO id allocation
	glBindBuffer( GL_ARRAY_BUFFER, m_nVboId );				// bind the ident with the mem location
	
	glBufferData(											// allocate the space in gfx memory
		GL_ARRAY_BUFFER_ARB,
		sizeof(CustomVertex) * m_nNumVertices,
		0,
		GL_STATIC_DRAW_ARB
	);
	
	glBufferSubData(										// copy vertex data to gfx mem
		GL_ARRAY_BUFFER,
		0,
		sizeof(CustomVertex) * m_nNumVertices,
		*m_ppVertices
	);

	// VBO for indices
	glGenBuffers( 1, &m_nVboIdIndex );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_nVboIdIndex );

	// allocate & memcpy
	glBufferData( 
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(DWORD) * m_nNumIndices,
		m_pIndices, 
		GL_STATIC_DRAW
	);

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}
	
void VertexBufferObject::Unload()
{
	m_bInitialized = false;

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );	

	if( m_nVboId )
		glDeleteBuffers( 1, &m_nVboId );

	if( m_nVboIdIndex )
		glDeleteBuffers( 1, &m_nVboIdIndex );


}
void VertexBufferObject::Draw()
{
	// TODO: CustomVertex offsets are hardcoded.  Bad thing.  Fix.
	if( ! m_bInitialized )
		return;

	glBindBuffer( GL_ARRAY_BUFFER,			m_nVboId );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	m_nVboIdIndex );
			
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );	

	glVertexPointer( 3, GL_FLOAT,	sizeof(CustomVertex), ((char*)NULL + 0) );
	glNormalPointer(GL_FLOAT,		sizeof(CustomVertex), ((char*)NULL + 12));
	glTexCoordPointer(2, GL_FLOAT,	sizeof(CustomVertex), ((char*)NULL + 20));	
	glColorPointer(3, GL_FLOAT,		sizeof(CustomVertex), ((char*)NULL + 32));	

	glDrawElements( GL_TRIANGLES, m_nNumIndices, GL_UNSIGNED_INT, ((char*)NULL +  0) );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );

	glBindBuffer( GL_ARRAY_BUFFER,			0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,	0 );
}