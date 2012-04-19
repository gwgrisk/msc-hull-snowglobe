
// out of time to complete the drift!!! Damn!

#pragma once

#include <GXBase.h>
#include "IGraphNode.h"

#include "Plane.h"
#include "Texture.h"
#include "Material.h"
#include <glm\glm.hpp>

#include <vector>
#include <string>
#include <fstream>

#include "Shader.h"
class SceneGraph;

class Drift :	public glex,
				public IGraphNode
{
private:
	bool			m_bInitialized;
	std::string		m_sConfigFile;
	std::string		m_sAlphaMap;

	Texture			m_AlphaMap;

	Plane			m_Plane;

	int				m_nRows;
	int				m_nCols;
	float			m_rCellWidth;
	float			m_rCellHeight;
	float			m_rCellDepth;

private:
	bool Initialize();
	bool LoadAssets();

public:
	Drift();
	Drift(  SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, const std::string & sConfigFile );
	Drift( const Drift & rhs );
	Drift & operator=( const Drift & rhs );
	~Drift();

	const bool Initialized() const { return m_bInitialized; }
	void Uninitialize();

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();

	// gets
	const std::string & AlphaMapFile() const	{ return m_sAlphaMap; }
	const int Rows() const						{ return m_nRows; } 
	const int Cols() const						{ return m_nCols; }
	const float CellWidth() const				{ return m_rCellWidth; }
	const float CellHeight() const				{ return m_rCellHeight; }
	const float CellDepth() const				{ return m_rCellDepth; }
	
	// sets
	void AlphaMapFile( const std::string & r )	{ m_sAlphaMap = r; }
	void Rows( const int n )					{ m_nRows = n; }
	void Cols( const int n )					{ m_nCols = n; }
	void CellWidth( const float r )				{ m_rCellWidth = r; }
	void CellHeight( const float r )			{ m_rCellHeight = r; }
	void CellDepth( const float r )				{ m_rCellDepth = r; }

};

std::ostream & operator << ( std::ostream & out, const Drift & r );
std::istream & operator >> ( std::istream & in, Drift & r );