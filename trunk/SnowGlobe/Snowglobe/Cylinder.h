
#pragma once

#include <glm\glm.hpp>
#include <vector>
#include "IGeometry.h"
#include "VertexClass.h"


class Cylinder : public IGeometry
{
private:
	bool						m_bInitialized;
		
	// args
	int							m_nStacks;
	int							m_nSlices;
	float						m_rStackHeight;
	float						m_rBaseRadius;
	float						m_rTipRadius;

	// internal
	int							m_nVertCount;
	int							m_nIndexCount;
	int							m_nCellCount;
	int							m_nTriCount;

	// representation
	CustomVertex *				_pNastyHackPtr;	
	unsigned short *			_pHackPtrIndices;

	std::vector<CustomVertex>	_vertices;
	std::vector<unsigned short>	_indices;

private:
	bool Initialize();
	
	// IGeometry
	bool GenerateData();
	void CreateRingVertices();
	void CreateStackIndices();

	void CreateBaseVertices();
	void CreateBaseIndices( const UINT nBaseIndex, const UINT nCenterIndex );

	void CreateTipVertices();	
	void CreateTipIndices( const UINT nTipIndex, const UINT nCenterIndex );

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
	const int VertsPerRing() const	{ return m_nSlices + 1; }
	const int VertCount() const		{ return m_nVertCount; }
	const int IndexCount() const	{ return m_nIndexCount; }
	const int TriCount() const		{ return m_nCellCount * 2; }

	CustomVertex** Vertices()		{ _pNastyHackPtr = &_vertices[0]; return &_pNastyHackPtr; }
	unsigned short* Indices()		{ _pHackPtrIndices = &(_indices.at(0)); return _pHackPtrIndices; }
};