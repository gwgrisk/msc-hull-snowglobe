
#pragma once

#include <GXBase.h>

#include <string>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>
#include "IGraphNode.h"
#include "Sphere.h"
#include "Material.h"
#include "Vbo.h"

class SceneGraph;
class Effect;
class CustomVertex;


class Light :	public IGraphNode,
				public glex
{
public:
	enum LightType { DirectionalLight, PointLight, SpotLight };

private:
	Sphere				m_Sphere;
	Effect*				m_pEffect;
	Material			m_material;
	Vbo<CustomVertex>*	m_pVbo;
	GLuint				m_nVaoId;

	LightType			m_Type;	

	glm::vec3			m_vIntensity;
	glm::vec4			m_vPosition;
	glm::vec3			m_vDirection;

	glm::vec3			m_La;
	glm::vec3			m_Ld;
	glm::vec3			m_Ls;

private:
	void Initialize();
	void Uninitialize();

	bool InitializeGeometry();
	void InitializeMaterial();
	bool InitializeTextures();
	bool SetPerVertexColour();
	bool InitializePosition();

	bool InitializeVbo( IGeometry & geometry );
	bool InitializeVao();
	bool GetShader();
	bool SetUniforms();

public:
	Light();
	Light(	SceneGraph * pGraph, IGraphNode* pParent, const std::string & sId );
	Light(	SceneGraph * pGraph, IGraphNode* pParent, const std::string & sId, 
			const glm::vec4 & pos, const glm::vec3 & La, const glm::vec3 & Ld, const glm::vec3 & Ls );
	Light( const Light & rhs );
	Light & operator=( const Light & rhs );	

	const LightType Type() const			{ return m_Type; }
	const glm::vec3 & Intensity() const		{ return m_Ld; }
	const glm::vec4 & Pos()	const			{ return m_vPosition; }
	const glm::vec3 & Direction() const		{ return m_vDirection; }
	const glm::vec3 & La() const			{ return m_La; }
	const glm::vec3 & Ld() const			{ return m_Ld; }
	const glm::vec3 & Ls() const			{ return m_Ls; }	

	void Type( const LightType & t )		{ m_Type = t; }
	void Intensity( const glm::vec3 & r )	{ m_Ld = r; }
	void Pos( const glm::vec4 & r )			{ m_vPosition = r; }
	void Direction( const glm::vec3 & r )	{ m_vDirection = r; }
	
	void La( const glm::vec3 & r )			{ m_La = r; }
	void Ld( const glm::vec3 & r )			{ m_Ld = r; }
	void Ls( const glm::vec3 & r )			{ m_Ls = r; }	

	void SetGraph( SceneGraph* pGraph )		{ this->m_pGraph = pGraph; }
	
public: 
	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();
};


// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const Light & r );
std::istream & operator >> ( std::istream & in, Light & r );