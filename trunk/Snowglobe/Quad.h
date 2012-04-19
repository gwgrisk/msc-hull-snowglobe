
// Quad
// Defines the geometry for a four point quad in the xz plane (hence the term quad).
// User specifies width and height, Quad generates vertices and indices 
// The default ctor will create a 10x10 quad.


#pragma once

#include <WTypes.h>
#include "IGeometry.h"
#include "VertexClass.h"

class Quad : public IGeometry
{
private:
	bool			m_bInitialized;
	CustomVertex *	m_pVertices;
	DWORD *			m_pdwIndices;

	float			m_rWidth;
	float			m_rHeight;

	int				m_nVertCount;
	int				m_nIndexCount;

private:
	bool Initialize();
	
	// IGeometry
	HRESULT GenerateVertices( CustomVertex** ppVertices );
	HRESULT GenerateIndices( DWORD** ppIndices );

public:
	Quad();
	Quad( const float rWidth, const float rHeight );
	Quad( const Quad & r );
	Quad & operator=( const Quad & r );
	~Quad();
		
	const bool Initialized() const	{ return m_bInitialized; }
	void Uninitialize();	

	// IGeometry
	const int VertCount() const		{ return m_nVertCount; }
	const int IndexCount() const	{ return m_nIndexCount; }
	const int TriCount() const		{ return m_nIndexCount / 3; }

	CustomVertex** Vertices()		{ return &m_pVertices; }
	DWORD* Indices()				{ return m_pdwIndices; }
};