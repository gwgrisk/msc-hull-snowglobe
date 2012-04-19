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


class Snowfall :	public glex,
					public IGraphNode
{
private:
	class SnowParticle2
	{
	public:
		bool	m_bInitialized;
		float** m_ppPosition;				//float [][3]
		float** m_ppVelocity;				//float [][3]
		float*	m_pParticleStartTime;
		float** m_ppInitialPosition;
		float** m_ppInitialVelocity;
			
	public:
		SnowParticle2() :
			m_bInitialized		(false),
			m_ppPosition		(NULL),
			m_ppVelocity		(NULL),
			m_pParticleStartTime(NULL),
			m_ppInitialPosition	(NULL),
			m_ppInitialVelocity	(NULL)
		{			
		}
		~SnowParticle2()
		{
			Uninitialize();
		}
		
		const bool Initialized() const { return m_bInitialized; }

		bool Initialize( int nParticleCount )
		{
			if( m_bInitialized )
				return m_bInitialized;

			m_ppInitialPosition		= (float**) new float[nParticleCount][3];
			m_ppInitialVelocity		= (float**) new float[nParticleCount][3];
			m_ppPosition			= (float**) new float[nParticleCount][3];
			m_ppVelocity			= (float**) new float[nParticleCount][3];	
			m_pParticleStartTime	= new float[nParticleCount];
			m_bInitialized			= true;

			return m_bInitialized;
		}
		void Uninitialize()
		{
			if( ! m_bInitialized )
				return;

			if( m_ppInitialPosition )
				delete [] m_ppInitialPosition;

			if( m_ppInitialVelocity )
				delete [] m_ppInitialVelocity;

			if( m_ppPosition )
				delete [] m_ppPosition;

			if( m_ppVelocity )
				delete [] m_ppVelocity;

			if( m_pParticleStartTime )
				delete [] m_pParticleStartTime;

			m_ppInitialPosition		= NULL;
			m_ppInitialVelocity		= NULL;
			m_ppPosition			= NULL;
			m_ppVelocity			= NULL;
			m_pParticleStartTime	= NULL;

			m_bInitialized			= false;
		}
	};
	class SnowParticle
	{
	public:
		float	m_rPosition[3];
		float	m_rVelocity[3];
		float	m_rParticleStartTime;
		float	m_rInitialPosition[3];
		float	m_rInitialVelocity[3];
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
	SnowParticle2				m_ParticleData2;

	float						m_rTimeElapsed;
	glm::vec3					m_vGravity;
	float						m_rParticleLifetime;
	
	std::vector<std::string>	m_sAssetFiles;
	std::vector<Texture*>		m_Textures;
	
	std::vector<Shader>			m_vShaders;
	std::vector<ShaderArg>		m_vShaderArgs;
	GLuint						m_nShaderProgId;
	
	GLuint						m_nElFlipYo;
	GLuint						m_nUpdateSub;
	GLuint						m_nRenderSub;
	GLfloat						m_rSecsDelta;

	GLuint						m_nParticleVBO[2];
	GLuint						m_nTransformFeedback[2];

	IGraphNode*					m_pGlobe;

private:
	bool Initialize();
	bool LoadAssets();
	bool GenerateSnowData();
	bool GenerateSnowData2();

	bool CreateShaderProgram();
	void DestroyShaderProgram();
	void CreateShaderArgs();
	void ApplyShaderArgs();
	void SetShaderUniforms();	

	void ElFlipYo() { m_nElFlipYo = 1 - m_nElFlipYo; }

	bool AssignUniformFloat( const std::string & sName, const float rVal );
	bool AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix );
	bool AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix );
	bool AssignUniformVec4( const std::string & sName, const glm::vec4 & vector );
	bool AssignUniformVec3( const std::string & sName, const glm::vec3 & vector );
	bool AssignUniformSampler2D( const std::string & sName, const GLuint nTexId );

public:
	Snowfall();
	Snowfall( SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, const std::string & sConfigFile );
	Snowfall( const Snowfall & rhs );
	Snowfall & operator=( const Snowfall & rhs );
	~Snowfall();

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

std::ostream & operator << ( std::ostream & out, const Snowfall & r );
std::istream & operator >> ( std::istream & in, Snowfall & r );
