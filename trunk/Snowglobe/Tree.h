
#pragma once

#include <GXBase.h>
#include <string>

#include "IGraphNode.h"
#include "LSystem.h"
#include "Cylinder.h"
#include "Texture.h"
#include "TreeShaderz.h"
#include "Material.h"
#include <vector>


class CustomVertex;
class ShaderProgram;
class SceneGraph;


class Tree :	public IGraphNode,
				public glex
{
private:
	typedef std::vector<Segment*>	Generation;
	typedef std::vector<Generation>	LTree;

private:
	bool					m_bInitialized;
	
	Material				m_material;
	Texture					m_texBark;
	Texture					m_texBump;
	std::string				m_sBarkFile;
	std::string				m_sBumpFile;

	eTreeShaders			m_CurrentShader;		// key select tree shader
	TreeShaderz				m_Shaderz;
	ShaderProgram *			m_pCurrentShader;

	float					m_rInitialSegLength;
	LSystem					m_LSystem;
	LTree					m_LTree;
	Cylinder				m_Cylinder;	

private:
	bool Initialize();
	void Uninitialize();

	bool InitializeLSystem();
	
	HRESULT InitializeLTree();
	void AddSegChildren( Segment* pSeg );
	void AddSegment( Segment* pSeg );

	bool SetPerVertexColour();
	bool LoadTexture();
	bool LoadBumpMap();
	
	bool InitializeShaders();
	bool CreateShaderProgram( const std::string & sVert, const std::string & sFrag, ShaderProgram ** ppShaderProg );
	void UninitializeShaders();

	void SetShaderArgs();

public:
	Tree();
	Tree(	SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId,
			const std::string & sBark, const std::string & sBump );
	Tree( const Tree & r );
	Tree & operator=( const Tree & r );
	~Tree();

	const bool Initialized() const	{ return m_bInitialized; }

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();
};