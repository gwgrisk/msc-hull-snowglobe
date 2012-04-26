
#pragma once

#include <GXBase.h>
#include <string>

#include "IGraphNode.h"
#include "Sphere.h"
#include "Texture.h"
#include "Material.h"
#include "Vbo.h"
#include <glm\glm.hpp>
#include "Effect.h"


class CustomVertex;
class SceneGraph;


class Globe :	public IGraphNode,
				public glex
{
private:
	bool					m_bInitialized;
	Sphere					m_Sphere;	

	Effect*					m_pEffect;
	Vbo<CustomVertex>*		m_pVbo;
	GLuint					m_nVaoId;

	Texture					m_tex;
	Material				m_material;
	std::string				m_sTexture;
	glm::vec4				m_vColour;

	float					m_rRadius;
	int						m_nStacks;
	int						m_nSlices;

private:
	Globe( const Globe & );						// disallow copy and assignment (for now.  Will add later;))
	Globe & operator=( const Globe & );

private:
	bool InitializeGeometry();
	void InitializeMaterial();
	bool InitializeTextures();
	bool SetPerVertexColour();
	bool InitializeVbo( IGeometry & geometry );
	bool InitializeVao();
	bool GetShader();
	bool SetUniforms();
	
	bool CreateShader();	
	void Uninitialize();

public:
	Globe();
	Globe(	SceneGraph * pGraph, IGraphNode* pParent, const std::string & sId, 
			float rRadius, int nSlices, int nStacks, const std::string & sTexture,
			const glm::vec4 & vColour );
	~Globe();
	
	bool Initialize();

	const bool Initialized() const	{ return m_bInitialized; }
	CustomVertex**	Vertices()		{ return m_Sphere.Vertices(); }
	const int ShaderProgId() const	{ return m_pEffect->Id(); } 

	
	// IGraphNode
	virtual HRESULT Update( const float & rSecsDelta );
	virtual HRESULT PreRender();
	virtual HRESULT Render();
	virtual HRESULT PostRender();
	virtual HRESULT DrawItem();
};