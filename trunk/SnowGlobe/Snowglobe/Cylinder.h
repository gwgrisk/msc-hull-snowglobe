
#pragma once

#include <glm\glm.hpp>
#include "IGeometry.h"

class CustomVertex;

class Cylinder : public IGeometry
{
private:
	bool					m_bInitialized;
		
	// args
	int						m_nStacks;
	int						m_nSlices;
	float					m_rStackHeight;
	float					m_rBaseRadius;
	float					m_rTipRadius;

	// internal
	int						m_nVertCount;
	int						m_nIndexCount;
	int						m_nCellCount;
	int						m_nTriCount;

	// representation
	CustomVertex *			m_pVertices;
	DWORD *					m_pdwIndices;

private:
	bool Initialize();
	
	// IGeometry
	HRESULT GenerateVertices( CustomVertex** ppVertices );
	HRESULT GenerateIndices( DWORD** ppdwIndices );

public:
	Cylinder();
	Cylinder( const int nSlices, const int nStacks, const float rStackHeight, const float rBaseRadius, const float rTipRadius );
	Cylinder( const Cylinder & r );
	Cylinder & operator=( const Cylinder & r );
	~Cylinder();
	
	const bool Initialized() const	{ return m_bInitialized; }
	void Uninitialize();

	const int Stacks() const		{ return m_nStacks; }
	const int Slices() const		{ return m_nSlices; }

	const float Length() const		{ return m_rStackHeight * m_nStacks; }
	const float BaseRadius() const	{ return m_rBaseRadius; }
	const float TipRadius() const	{ return m_rTipRadius; }


	// IGeometry
	const int VertCount() const		{ return m_nVertCount; }
	const int IndexCount() const	{ return m_nIndexCount; }
	const int TriCount() const		{ return m_nCellCount * 2; }

	CustomVertex** Vertices()		{ return &m_pVertices; }	
	DWORD* Indices()				{ return m_pdwIndices; }
};