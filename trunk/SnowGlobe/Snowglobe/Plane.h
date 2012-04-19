
// Geometry class to generate a 2D plane of vertices and associated indices

#pragma once
#include <vector>
#include <glm\glm.hpp>
#include "IGeometry.h"
class CustomVertex;

class Plane : public IGeometry
{
private:
	CustomVertex *					m_pVertices;
	DWORD *							m_pdwIndices;
	std::vector<glm::vec3>			m_FaceNormals;

	int								m_nVertCols;
	int								m_nVertRows;
	int								m_nVertCount;
	int								m_nIndexCount;

	int								m_nCellRows;
	int								m_nCellCols;
	float							m_rWidth;
	float							m_rHeight;


	float							m_rCellWidth;
	float							m_rCellHeight;
	float							m_rCellDepth;	
	int								m_nCellCount;
	int								m_nTriCount;

	bool							m_bInitialized;
	
private:
	bool Initialize();	

	HRESULT CreatePlane( 
		const int			nVertRows, 
		const int			nVertCols, 
		const float			rCellWidth, 
		const float			rCellHeight, 
		CustomVertex**		ppVertices,
		DWORD**				ppdwIndices
	);

	HRESULT GenerateVertices( CustomVertex** ppVertices );
	HRESULT GenerateIndices( DWORD** ppdwIndices );
	HRESULT GenerateNormals( CustomVertex** ppVertices );
	
	void ComputeFaceNormals();
	void IdentifyFacesForVertex( std::vector<int> & vnFaces, int nVertIndex );
	void ComputeVNormalFromFaceList( std::vector<int> & vnFaces, glm::vec3 & vNormal );

public:

	Plane();
	Plane( int nVertCols, int nVertRows, const float rCellWidth = 5.0f, const float rCellHeight = 5.0f, const float rCellDepth = 5.0f );
	Plane( const std::string & sHeightField, const std::string & sTexture, const float rCellWidth = 5.0f, const float rCellHeight = 5.0f, const float rCellDepth = 5.0f );	
	Plane( const Plane & rhs );
	Plane & operator=( const Plane & rhs );
	~Plane();

	void Uninitialize();	

	bool Initialized() const		{ return m_bInitialized; }
	const int VertCount() const		{ return m_nVertCount; }
	const int IndexCount() const	{ return m_nIndexCount; }
	const int TriCount() const		{ return m_nCellCount * 2; }

	CustomVertex** Vertices()		{ return &m_pVertices; }
	DWORD* Indices()				{ return m_pdwIndices; }
		
	void RecalcNormalMap();
};