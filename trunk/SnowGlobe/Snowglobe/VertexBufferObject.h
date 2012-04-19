
#pragma once
#include <GXBase.h>

class CustomVertex;

class VertexBufferObject : public glex
{
private:
	GLuint			m_nVboId;
	GLuint			m_nVboIdIndex;
	bool			m_bInitialized;

	int				m_nNumVertices;
	int				m_nNumIndices;
	CustomVertex**	m_ppVertices;	
	DWORD*			m_pIndices;

private:
	void Initialize();
	void CopyToGfxMem();

	VertexBufferObject( const VertexBufferObject & rhs );				// disallow copy
	VertexBufferObject & operator=(const VertexBufferObject & rhs );	// disallow assignment

public:
	VertexBufferObject();
	VertexBufferObject( int nNumVerts, CustomVertex** ppVertices, int nNumIndices, DWORD* pIndices );
	~VertexBufferObject();
		
	bool Initialized() const;
	void Draw();
	void Unload();

	GLuint VboId() const		{ return m_nVboId; }
	GLuint VboIndexId() const	{ return m_nVboId; }

	int NumVertices() const		{ return m_nNumVertices; }
	int NumIndices() const		{ return m_nNumIndices; }
};