
#pragma once

#include <GXBase.h>
#include "IGraphNode.h"
#include "Plane.h"
#include "Quad.h"
#include "Texture.h"
#include <glm\glm.hpp>
#include <string>
#include "Material.h"

class CustomVertex;
class ShaderProgram;
class SceneGraph;


class Pond :	public IGraphNode,
				public glex
{
private:
	bool				m_bInitialized;

	// not sure which primitive to go with yet.  Both quad and plane are candidates.
	// it depends whether I choose to animate vertices for the water, or do everything
	// in the shader.
	Plane				m_Plane;
	Quad				m_Quad;

	ShaderProgram *		m_pShaderProgram;

	Texture				m_TextureMap; 
	Texture				m_AlphaMap;
	Texture				m_NormalMap;

	Material			m_material;

	std::string			m_sTextureMap;
	std::string			m_sAlphaMap;
	std::string			m_sNormalMap;
	float				m_rWidth;
	float				m_rHeight;

private:
	bool CreateShader( ShaderProgram** ppShader, IGeometry & pShape );
	void SetShaderArgs( ShaderProgram* pShader, Texture* pTex, Texture* pAlpha, const Material & mat );

	bool Initialize();
	void Uninitialize();

public:
	Pond();	
	Pond(	SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, 
			const float rWidth, const float rHeight, 
			const std::string & sTexture,
			const std::string & sAlphaMap,
			const std::string & sNormalMap );
	Pond( const Pond & r );
	Pond & operator=( const Pond & r );
	~Pond();	

	const bool Initialized() const	{ return m_bInitialized; }
	CustomVertex**	Vertices()		{ return m_Plane.Vertices(); }

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();
};