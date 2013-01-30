
#pragma once

#include <GXBase.h>
#include "IGraphNode.h"

#include "Quad.h"
#include "Tri.h"
#include "Texture.h"
#include "Material.h"
#include "Vbo.h"
#include "VertexClass.h"
#include <glm\glm.hpp>

#include <vector>
#include <string>
#include <fstream>


class Effect;
class SceneGraph;

enum HouseAssets	{ WallTexture, RoofTexture, WallBump, RoofBump, NumAssets };
enum HouseWalls		{ front, right, back, left, NumWalls };
enum HouseRoofSlopes{ FrontSlope, BackSlope, NumRoofSlopes };
enum HouseRoofSides	{ LeftSide, RightSide, NumRoofSides };
enum HouseParts		{ FrontWall, RightWall, BackWall, LeftWall, FrontRoofSlope, BackRoofSlope, RightRoofSide, LeftRoofSide, NumHouseParts };

class House :	public IGraphNode,
				public glex
{
private:
	bool						m_bInitialized;

	Quad						m_Quad;
	Tri							m_Tri;

	std::vector<Texture*>		m_Textures;
	Material					m_matWall;
	Material					m_matRoof;

	std::vector<glm::mat4>		m_mW;

	Effect*						_pQuadEffect;
	Effect*						_pTriEffect;
	Vbo<CustomVertex> *			_pQuadVbo;
	Vbo<CustomVertex> *			_pTriVbo;
	GLuint						_nQuadVao;
	GLuint						_nTriVao;

	GLuint						m_nSunSub;
	GLuint						m_nSpotlightSub;

	std::string					m_sConfigFile;
	std::vector<std::string>	m_sAssetFiles;
	
private:
	bool Initialize();
	bool LoadAssets();
	void TransformGeometry();

	bool InitializeGeometry();
	bool InitializeTextures();
	bool InitializeVbo( Vbo<CustomVertex> **ppVbo, IGeometry & geometry );
	bool InitializeVao( Effect* pEffect, GLuint & nVaoId, Vbo<CustomVertex>* pVbo );

	bool GetShader( Effect** ppEffect );
	bool SetShaderArgs( Effect* pEffect, const GLuint nVaoId, const Vbo<CustomVertex>* pVbo, const Texture* pTex, const Texture* pBump, const Material & mtl );

	bool DrawPrimitive( Effect* pEffect, IGeometry* pShape, const glm::mat4 & mW );

public:
	House();
	House( SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, const std::string & sConfigFile );
	House( const House & rhs );
	House & operator=( const House & rhs );
	~House();

	const bool Initialized() const { return m_bInitialized; }
	void Uninitialize();

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();

	// gets
	const std::vector<std::string> & AssetFiles() const		{ return m_sAssetFiles; }
	const Material & RoofMtl() const						{ return m_matRoof; }
	const Material & WallMtl() const						{ return m_matWall; }

	// Sets
	void AssignAssets( 
		const std::vector<std::string> & sAssets,		
		const Material & matWalls, 
		const Material & matRoof 
	);
};

std::ostream & operator << ( std::ostream & out, const House & r );
std::istream & operator >> ( std::istream & in, House & r );