
#pragma once

#include <GXBase.h>
#include <string>

#include "IGraphNode.h"
#include "LSystem.h"
#include "Cylinder.h"
#include "Texture.h"
#include "Material.h"
#include "Vbo.h"
#include <vector>
#include <glm\glm.hpp>

class Segment;
class Effect;

class Tree :	public IGraphNode,
				public glex
{
	friend class Segment;

private:	
	typedef std::vector<Segment*>	Generation;
	typedef std::vector<Generation>	LTree;
	enum							eTreeShader { WireFrame, Flat, Smooth, SmoothTextured, BumpTextured };
	std::vector<std::string>		_ShaderNames;

private:
	bool							m_bInitialized;
	
	Material						m_material;
	Texture							m_texBark;
	Texture							m_texBump;
	std::string						m_sBarkFile;
	std::string						m_sBumpFile;
	
	eTreeShader						m_CurrentShader;	
	Effect*							m_pEffect;		// currently selected effect
	GLuint							m_nSunSub;
	GLuint							m_nSpotlightSub;
	Vbo<CustomVertex> *				m_pVbo;			// vertex data
	GLuint							m_nVaoId;
	
	
	LTree							m_LTree;
	LSystem							m_LSystem;	
	Cylinder						m_Cylinder;	
	float							m_rInitialSegLength;

private:
	bool Initialize();
	void Uninitialize();	

	void InitShaderNames();
	bool InitializeGeometry();
	bool InitializeMtl();
	bool InitializeTextures();
	bool InitializeVbo( IGeometry & geometry );
	bool InitializeVao();
	bool InitializePerVertexColour( const glm::vec4 & col );

	bool GetShader(const eTreeShader e);
	bool SetShaderArgs();
	void SelectShaderSubroutine( GLenum shadertype = GL_FRAGMENT_SHADER );

	bool InitializeLSystem();	
	HRESULT InitializeLTree();
	void AddSegChildren( Segment* pSeg );
	void AddSegment( Segment* pSeg );	

	void RenderSegment( Segment* pSeg );

	glm::mat4 CalcSegOrientationMatrix(const glm::vec3 & vOrientation, const glm::vec3 & vPos );

public:
	Tree();
	Tree(	SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId,
			const std::string & sBark, const std::string & sBump );
	Tree( const Tree & r );
	Tree & operator=( const Tree & r );
	~Tree();

	const bool Initialized() const	{ return m_bInitialized; }

	float CylinderLength() { return m_Cylinder.Length(); }

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();
};