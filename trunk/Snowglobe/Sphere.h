
#pragma once

#include <glm\glm.hpp>
#include "IGeometry.h"

class CustomVertex;

class Sphere : public IGeometry
{
private:
	bool					m_bInitialized;
		
	// args
	int						m_nStacks;
	int						m_nSlices;
	float					m_rRadius;	

	// internal
	int						m_nVertCount;	
	int						m_nIndexCount;

	// representation
	CustomVertex *			m_pVertices;
	DWORD *					m_pdwIndices;


private:
	bool Initialize();	

	// IGeometry
	HRESULT GenerateVertices( CustomVertex** ppVertices );	
	HRESULT GenerateIndices( DWORD ** ppdwIndices );

public:
	Sphere();
	Sphere( const float rRadius, const int nSlices, const int nStacks );
	Sphere( const Sphere & rhs );	
	Sphere & operator=( const Sphere & rhs );
	~Sphere();

	const bool Initialized() const	{ return m_bInitialized; }
	void Uninitialize();

	// IGeometry
	const int VertCount() const		{ return m_nVertCount; }
	const int IndexCount() const	{ return m_nIndexCount; }
	const int TriCount() const		{ return m_nStacks * m_nSlices * 2; }
	const int Slices() const		{ return m_nSlices; }
	const int Stacks() const		{ return m_nStacks; }

	CustomVertex** Vertices()		{ return &m_pVertices; }
	DWORD* Indices()				{ return m_pdwIndices; }
};