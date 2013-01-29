
// cheap to copy types ( int, char, float etc ) pass by value 
// http://cpp-next.com/archive/2009/08/want-speed-pass-by-value/


#pragma once
#include <GXBase.h>
#include <string>
#include <vector>

#include <AntiMatter\math.h>
#include "IGraphNode.h"
#include "Plane.h"
#include "Texture.h"
#include "Vbo.h"
#include "VertexClass.h"
#include "Material.h"

class Effect;


class Terrain : public IGraphNode,
				public glex
{
public:
	// enum	BitDepth { Bpp0, Bpp8 = 8, Bpp16 = 16, Bpp24 = 24, Bpp32 = 32 };

private:
	bool					m_bInitialized;
	std::string				m_sHeightField;
	std::string				m_sTexture;
	std::string				m_sAlphaMap;
	
	Material				m_material;
	Texture					m_texGrass;
	Texture					m_texAlphaMap;

	GLuint					m_nTextureId;

	int						m_nVertCols;
	int						m_nVertRows;
	float					m_rCellWidth;
	float					m_rCellHeight;
	float					m_rCellDepth;
	
	Plane					m_Plane;	
	Effect*					m_pEffect;
	Vbo<CustomVertex> *		m_pVbo;			// vertex data
	GLuint					m_nVaoId;
	GLuint					m_nSunSub;
	GLuint					m_nSpotlightSub;

	bool					m_bOldShader;

private:		
	void ApplyHeightField();
	
	void SetVertexColours();

	bool GetShader();	
	void SetShaderArgs();

	bool InitializeMtl();
	bool InitializeGeometry();
	bool InitializeTextures();
	bool InitializeVbo( IGeometry & geometry );
	bool InitializeVao();

public:
	
	Terrain();

	Terrain(
		SceneGraph*			pGraph, 
		IGraphNode*			pParent, 
		const std::string & sId, 
		const int			nVertCols, 
		const int			nVertRows, 
		const float			rCellWidth	= 1.0f, 
		const float			rCellHeight = 1.0f, 
		const float			rCellDepth	= 1.0f
	);

	Terrain(	
		SceneGraph*			pGraph, 
		IGraphNode*			pParent, 
		const std::string & sId, 
		const std::string & sHeightField, 
		const std::string & sTexture,
		const std::string & sAlphaMap,
		const int			nVertCols, 
		const int			nVertRows, 
		const float			rCellWidth	= 1.0f, 
		const float			rCellHeight = 1.0f, 
		const float			rCellDepth	= 1.0f 
	);

	Terrain( const Terrain & rhs );
	Terrain & operator=( const Terrain & rhs );	
	
	~Terrain();	

	bool Initialize();
	void Uninitialize();

	bool Initialized() const	{ return m_bInitialized; }
	int TriCount() const		{ return m_Plane.TriCount(); }

	void Draw();

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();
};