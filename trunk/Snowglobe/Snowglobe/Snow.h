
#pragma once

#include <GXBase.h>
#include "IGraphNode.h"

#include "Texture.h"
#include <glm\glm.hpp>

#include <vector>
#include <string>
#include <fstream>

#include "Shader.h"
class SceneGraph;
class Globe;


class Snow :	public glex,
				public IGraphNode
{
private:
	class SnowParticle
	{
	public:
		float	m_rInitialPosition[3];
		float	m_rInitialVelocity[3];
		float	m_rStartTime;
		float	m_rPadding;
	};
	class ShaderArg
	{
	public:	
		std::string		sFieldName;
		GLuint			nFieldSize;		// not bytes, number of components
		GLubyte			nFieldOffset;
		GLuint			nStride;
	};

private:
	bool						m_bInitialized;
	std::string					m_sConfigFile;

	float						m_rParticleFieldRadius;
	unsigned int				m_nParticleCount;
	SnowParticle*				m_pParticleData;

	float						m_rTimeElapsed;
	glm::vec3					m_vGravity;
	float						m_rParticleLifetime;
	
	std::vector<std::string>	m_sAssetFiles;
	std::vector<Texture*>		m_Textures;
	
	std::vector<Shader>			m_vShaders;
	std::vector<ShaderArg>		m_vShaderArgs;
	GLuint						m_nShaderProgId;
	GLuint						m_nVboId;

	IGraphNode*					m_pGlobe;

private:
	bool Initialize();
	bool LoadAssets();
	bool GenerateSnowData();

	bool CreateShader();
	void CreateShaderArgs();
	void SetShaderUniforms();	

	bool AssignUniformFloat( const std::string & sName, const float rVal );
	bool AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix );
	bool AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix );
	bool AssignUniformVec4( const std::string & sName, const glm::vec4 & vector );
	bool AssignUniformVec3( const std::string & sName, const glm::vec3 & vector );
	bool AssignUniformSampler2D( const std::string & sName, const GLuint nTexId );

public:
	Snow();
	Snow( SceneGraph* pGraph, IGraphNode* pParent, IGraphNode* pGlobe, const std::string & sId, const std::string & sConfigFile );
	Snow( const Snow & rhs );
	Snow & operator=( const Snow & rhs );
	~Snow();

	const bool Initialized() const { return m_bInitialized; }
	void Uninitialize();

	// IGraphNode
	HRESULT Update( const float & rSecsDelta );
	HRESULT PreRender();
	HRESULT Render();
	HRESULT PostRender();
	HRESULT DrawItem();

	// gets
	const int ParticleCount() const					{ return m_nParticleCount; }
	const float ParticleFieldRadius() const			{ return m_rParticleFieldRadius; }
	const float ParticleLifetime() const			{ return m_rParticleLifetime; }
	const std::vector<std::string> Assets() const	{ return m_sAssetFiles; }

	// sets
	void ParticleCount( const int n )				{ m_nParticleCount = n; }
	void ParticleFieldRadius( const float r )		{ m_rParticleFieldRadius = r; }
	void ParticleLifetime( const float r )			{ m_rParticleLifetime = r; }

	// stream support
	void AssignAssets( const std::vector<std::string> & sAssets );
};

std::ostream & operator << ( std::ostream & out, const Snow & r );
std::istream & operator >> ( std::istream & in, Snow & r );