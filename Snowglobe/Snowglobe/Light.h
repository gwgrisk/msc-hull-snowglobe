
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
#include <AntiMatter\precision.h>

class SceneGraph;
class Effect;
class CustomVertex;


class Light :	public IGraphNode,
				public glex
{
public:
	// sun			- directional
	// bulb			- point
	// spotlight	- em, spot!
	enum LightType { DirectionalLight, PointLight, SpotLight };

private:
	Sphere				m_Sphere;
	Effect*				m_pEffect;
	Material			m_material;
	Vbo<CustomVertex>*	m_pVbo;
	GLuint				m_nVaoId;

	LightType			m_Type;	

	glm::vec4			m_vPosition;
	glm::vec3			m_La;
	glm::vec3			m_Ld;
	glm::vec3			m_Ls;

	glm::vec3			m_vDirection;
	real				m_rExponent;
	real				m_rCutOff;


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
	Light(
		SceneGraph *		pGraph, 
		IGraphNode*			pParent, 
		const std::string & sId, 

		const glm::vec4 &	pos, 
		const glm::vec3 &	La, 
		const glm::vec3 &	Ld, 
		const glm::vec3 &	Ls,
		const LightType		lt		= LightType::DirectionalLight,
		const glm::vec3 &	vDir	= glm::vec3(0,0,0), 
		const real			exp		= 0.0f, 
		const real			rCutoff = 0.0f
	);
	Light( const Light & rhs );
	Light & operator=( const Light & rhs );	
		
	const glm::vec4 & Pos()	const			{ return m_vPosition; }	
	const glm::vec3 & La() const			{ return m_La; }
	const glm::vec3 & Ld() const			{ return m_Ld; }
	const glm::vec3 & Ls() const			{ return m_Ls; }	
	const LightType Type() const			{ return m_Type; }
	const glm::vec3 & Direction() const		{ return m_vDirection; }
	const real Exponent() const				{ return m_rExponent; }
	const real CutOff() const				{ return m_rCutOff; }
		
	void Pos( const glm::vec4 & r )			{ m_vPosition	= r; }
	void La( const glm::vec3 & r )			{ m_La			= r; }
	void Ld( const glm::vec3 & r )			{ m_Ld			= r; }
	void Ls( const glm::vec3 & r )			{ m_Ls			= r; }	
	void Type( const LightType & t )		{ m_Type		= t; }
	void Direction( const glm::vec3 & r )	{ m_vDirection	= r; }
	void Exponent( const real r )			{ m_rExponent	= r; }
	void CutOff( const real r )				{ m_rCutOff		= r; }
		
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