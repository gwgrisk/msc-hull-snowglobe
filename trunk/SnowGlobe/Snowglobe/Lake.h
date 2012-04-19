
#pragma once

#include <GXBase.h>
#include "IGraphNode.h"

#include "Vbo.h"

#include "Plane.h"
#include "Quad.h"

#include "EffectMgr.h"
#include "Texture.h"
#include "Material.h"

#include <glm\glm.hpp>
#include <string>

class CustomVertex;
class SceneGraph;

class Lake : public IGraphNode,
			 public glex
{
private:
	bool					m_bInitialized;
	
	Plane					m_Plane;
	Quad					m_Quad;

	Effect*					m_pEffect;
	Vbo<CustomVertex>*		m_pVbo;
	GLuint					m_nVaoId;

	Texture					m_TextureMap; 
	Texture					m_AlphaMap;
	Texture					m_NormalMap;
	Material				m_material;

	std::string				m_sTextureMap;
	std::string				m_sAlphaMap;
	std::string				m_sNormalMap;

	float					m_rWidth;
	float					m_rHeight;

	int						m_nNumWaves;	

	std::vector<float>		m_rAmplitude;
	std::vector<float>		m_rWavelength;
	std::vector<float>		m_rVelocity;
	std::vector<glm::vec2>	m_vDirection;

private:
	bool Initialize();
	void Uninitialize();

	bool InitializeGeometry();
	bool InitializeMaterial();
	bool InitializeTextures();
	bool InitializeVbo( IGeometry & geometry );
	bool InitializeVao();
	bool InitializeWaveData();

	bool GetShader();

	void SetAttrs();
	void SetUniforms();	

public:
	Lake();	
	Lake(	SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, 
			const float rWidth, const float rHeight, 
			const std::string & sTexture,
			const std::string & sAlphaMap,
			const std::string & sNormalMap );
	Lake( const Lake & r );
	Lake & operator=( const Lake & r );
	~Lake();

	const bool Initialized() const	{ return m_bInitialized; }
	CustomVertex**	Vertices()		{ return m_Plane.Vertices(); }

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();
};