
// Tri
// Defines the geometry for a three point triangle (hence the name).
// User specifies x, y, z, Tri generates vertices and indices 

#pragma once

#include <WTypes.h>
#include "IGeometry.h"
#include "VertexClass.h"

class Tri : public IGeometry
{
private:
	bool				m_bInitialized;
	CustomVertex *		m_pVertices;
	unsigned short *	m_pdwIndices;

	float				m_rBaseLength;
	float				m_rHeight;

	int					m_nVertCount;
	int					m_nIndexCount;

private:
	bool Initialize();
	
	// IGeometry
	bool GenerateData(){ return true; }
	HRESULT GenerateVertices( CustomVertex** ppVertices );
	HRESULT GenerateIndices( unsigned short** ppIndices );

public:
	Tri();
	Tri( const float rBaseLength, const float rHeight );
	Tri( const Tri& r );
	Tri & operator=( const Tri & r );
	~Tri();
		
	const bool Initialized() const	{ return m_bInitialized; }
	void Uninitialize();	

	// IGeometry
	const int VertCount() const		{ return m_nVertCount; }
	const int IndexCount() const	{ return m_nIndexCount; }
	const int TriCount() const		{ return m_nIndexCount / 3; }

	CustomVertex** Vertices()		{ return &m_pVertices; }
	unsigned short* Indices()		{ return m_pdwIndices; }
};