
#pragma once

#include <GXBase.h>
#include <string>

#include "IGraphNode.h"
#include "Sphere.h"
#include "Texture.h"
#include "Material.h"
#include <glm\glm.hpp>
#include "ShaderProgram.h"

class CustomVertex;
class SceneGraph;


class Globe :	public IGraphNode,
				public glex
{
private:
	bool			m_bInitialized;
	Sphere			m_Sphere;	
	ShaderProgram*	m_pShaderProgram;
	Texture			m_tex;
	Material		m_material;
	std::string		m_sTexture;
	glm::vec4		m_vColour;

	float			m_rRadius;
	int				m_nStacks;
	int				m_nSlices;

	float			m_rRotYAngle;
	float			m_rRotYStep;
	float			m_rRotYSpeed;

private:
	Globe( const Globe & );						// disallow copy and assignment (for now.  Will add later;))
	Globe & operator=( const Globe & );

private:
	bool SetPerVertexColour();
	bool SetTexture();
	bool CreateShader();
	void SetShaderArgs();
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
	const int ShaderProgId() const	{ return m_pShaderProgram ? m_pShaderProgram->ShaderProgId() : -1; }

	
	// IGraphNode
	virtual HRESULT Update( const float & rSecsDelta );
	virtual HRESULT PreRender();
	virtual HRESULT Render();
	virtual HRESULT PostRender();
	virtual HRESULT DrawItem();
	
};