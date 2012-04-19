
#pragma once
#include <GXBase.h>

template <typename T>
class Vbo : public glex
{
private:
	GLuint			m_nVboId;
	GLuint			m_nVboIdIndex;
	bool			m_bInitialized;

	int				m_nNumVertices;
	int				m_nNumIndices;
	T**				m_ppVertices;
	DWORD*			m_pIndices;

private:
	void Initialize();
	bool CopyToGfxMem();

public:
	Vbo();
	Vbo( int nNumVerts, T** ppVertices, int nNumIndices, DWORD* pIndices );
	Vbo( const Vbo<T> & rhs );						// copy
	Vbo<T> & operator=( const Vbo<T> & rhs );		// assignment
	~Vbo();
	
	bool Bind();
	void Unbind();
	void Uninitialize();

	bool Initialized() const	{ return m_bInitialized; }
	GLuint Id() const			{ return m_nVboId; }
	GLuint IndexId() const		{ return m_nVboIdIndex; }

	int NumVertices() const		{ return m_nNumVertices; }
	int NumIndices() const		{ return m_nNumIndices; }
};

/*
Template function definitions need to be in the header (or you can use more exotic features like explicit template instantiation). 
This is because a template function is only understood as actual code when it is used, and I assume your use is not in the same translation unit as your definition.
http://stackoverflow.com/questions/9366388/unresolved-external-symbol-when-invoking-a-template-ctor/
*/

template <typename T>
Vbo<T>::Vbo() :	
	m_bInitialized	( false ),
	m_nVboId		( 0 ),
	m_nVboIdIndex	( 0 ),	
	m_nNumVertices	( 0 ),
	m_nNumIndices	( 0 ),
	m_ppVertices	( NULL ),
	m_pIndices		( NULL )
{
}
		
template <typename T>
Vbo<T>::Vbo( int nNumVerts, T** ppVertices, int nNumIndices, DWORD* pIndices ) :	
	m_bInitialized	( false ),	
	m_nVboId		( 0 ),
	m_nVboIdIndex	( 0 ),	
	m_nNumVertices	( nNumVerts ),
	m_nNumIndices	( nNumIndices ),
	m_ppVertices	( ppVertices ),
	m_pIndices		( pIndices )
{
	glex::Load();
	Initialize();
}

template <typename T>
Vbo<T>::Vbo( const Vbo<T> & rhs ) :	
	m_nVboId		( 0 ),
	m_nVboIdIndex	( 0 ),
	m_bInitialized	( false ),
	m_nNumVertices	( rhs.m_nNumVertices ),
	m_nNumIndices	( rhs.m_nNumIndices ),
	m_ppVertices	( rhs.m_ppVertices ),
	m_pIndices		( rhs.m_pIndices )
{
	glex::Load();
	Initialize();
}
	
template <typename T>
Vbo<T> & Vbo<T>::operator= ( const Vbo<T> & rhs )
{
	if( this != &rhs )
	{
		Uninitialize();

		m_nNumVertices	= rhs.m_nNumVertices;
		m_nNumIndices	= rhs.m_nNumIndices;
		m_ppVertices	= rhs.m_ppVertices;
		m_pIndices		= rhs.m_pIndices;

		Initialize();
	}

	return *this;
}

template <typename T>
Vbo<T>::~Vbo()
{
	Uninitialize();	
}

template <typename T>
void Vbo<T>::Initialize()
{	
	if( (m_nNumVertices <= 0) || (! *m_ppVertices) )
		return;	

	m_bInitialized = CopyToGfxMem();
	return;
}
	
template <typename T>
void Vbo<T>::Uninitialize()
{
	if( ! m_bInitialized )
		return;

	m_bInitialized = false;

	Unbind();

	if( m_nVboId )
	{
		glDeleteBuffers( 1, &m_nVboId );
		m_nVboId = 0;
	}

	if( m_nVboIdIndex )
	{
		glDeleteBuffers( 1, &m_nVboIdIndex );
		m_nVboIdIndex = 0;
	}
}

template <typename T>
bool Vbo<T>::CopyToGfxMem()
{
	using AntiMatter::AppLog;

	glGenBuffers( 1, &m_nVboId );							// request a VBO id allocation
	glBindBuffer( GL_ARRAY_BUFFER, m_nVboId );				// bind the ident with the mem location
	
	glBufferData(											// allocate the space in gfx memory
		GL_ARRAY_BUFFER,
		sizeof(T) * m_nNumVertices,
		0,
		GL_STATIC_DRAW
	);
	
	if( glGetError() != GL_NO_ERROR )
	{
		AppLog::Ref().LogMsg("%s glBufferData call failed", __FUNCTION__);
		return false;
	}

	
	// copy data to gfx card
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		sizeof(T) * m_nNumVertices,
		*m_ppVertices
	);

	if( glGetError() != GL_NO_ERROR )
	{
		AppLog::Ref().LogMsg("%s glBufferSubData call failed", __FUNCTION__);
		return false;
	}


	// VBO for indices
	if( m_nNumIndices > 0 )
	{
		glGenBuffers( 1, &m_nVboIdIndex );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_nVboIdIndex );

		// allocate & memcpy
		glBufferData( 
			GL_ELEMENT_ARRAY_BUFFER,
			sizeof(DWORD) * m_nNumIndices,
			m_pIndices, 
			GL_STATIC_DRAW
		);

		if( glGetError() != GL_NO_ERROR )
		{
			AppLog::Ref().LogMsg("%s glBufferData on index vbo failed", __FUNCTION__);
			return false;
		}
	}

	return true;
}

template <typename T>
bool Vbo<T>::Bind()
{
	if( ! m_bInitialized )
		return false;

	if( m_nVboId == 0 && m_nVboIdIndex == 0 )
		return false;

	glBindBuffer( GL_ARRAY_BUFFER, m_nVboId );

	if( m_nVboIdIndex )
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_nVboIdIndex );

	return true;
}

template <typename T>
void Vbo<T>::Unbind()
{
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}
