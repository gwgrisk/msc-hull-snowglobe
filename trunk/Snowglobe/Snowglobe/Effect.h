
#pragma once

#include <GXBase.h>
#include <string>
#include <vector>
#include <fstream>
#include <glm\glm.hpp>

class Shader;
class ShaderDesc;
class ShaderInputAttribute;


class Effect : public glex
{
public:
	enum EffectBuildState { Uncompiled, Compiled, Linked };

private:
	bool									m_bInitialized;
	bool									m_bAutoBuild;

	std::string								m_sEffectName;
	GLuint									m_nProgramId;

	std::vector <Shader>					m_Shaders;
	std::vector <ShaderDesc>				m_Descs;
	std::vector <ShaderInputAttribute>		m_Attrs;
	EffectBuildState						m_BuildState;

private:
	bool Initialize(	const std::vector<ShaderDesc> &				Descs, 
						const std::vector<ShaderInputAttribute> &	Attrs );
	void Uninitialize();

public:
	Effect();
	Effect( const std::vector<ShaderDesc> & Descs, const std::vector<ShaderInputAttribute> & Attrs, const std::string & sName, const bool bAutoBuild = true );
	Effect( const Effect & r );
	Effect & operator=( const Effect & r );
	~Effect();

	const bool Initialized() const					{ return m_bInitialized; }
	const GLuint Id() const							{ return m_nProgramId; }
	const std::string Name() const					{ return m_sEffectName; }
	const enum EffectBuildState BuildState() const	{ return m_BuildState; }

	const std::vector<ShaderInputAttribute> & Attributes() const { return m_Attrs; }

	bool Build();
	bool SetInputAttributes();
	bool CompileShaders();
	bool Link();
		
	// these need to be class functions, in order to get gl functions from glex
	bool AssignUniformInt( const std::string & sName, const int rVal );
	bool AssignUniformFloat( const std::string & sName, const float rVal );
	bool AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix );
	bool AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix );
	bool AssignUniformVec4( const std::string & sName, const glm::vec4 & vector );
	bool AssignUniformVec3( const std::string & sName, const glm::vec3 & vector );	
	bool AssignUniformVec2( const std::string & sName, const glm::vec2 & vector );
	bool AssignUniformSampler2D( const std::string & sName, const GLuint nTexId );
};

//std::ofstream & operator << ( std::ofstream & out, const Effect & r );
//std::ifstream & operator >> ( std::ifstream & in, Effect & r );