
// To create a shader program, the user should create a ShaderDesc vector and 
// populate one per shader (e.g. Vertex, Fragment etc).
//
// To pass args into the shader, the user should populate a vector of ShaderArgs
//

#pragma once
#include <GXBase.h>
#include <glex.h>
#include <vector>
#include <string>

#include <glm\glm.hpp>

class CustomVertex;
class ShaderDesc;
class ShaderInputAttribute;
class Shader;

class ShaderProgram : public glex
{
private:
	GLuint					m_nProgramId;
	std::vector<Shader>		m_Shaders;	
	bool					m_bInitialized;
	
	GLuint					m_nVaoId;
	GLuint					m_nVboId;
	GLuint					m_nVboIndexId;

	CustomVertex**			m_ppVertices;
	unsigned short*			m_pdwIndices;

	GLuint					m_nVertexCount;
	GLuint					m_nIndexCount;

private:
	ShaderProgram( const ShaderProgram & r );					// disallow copy
	ShaderProgram & operator=( const ShaderProgram & r );		// disallow assignment

	bool Initialize(
		const std::vector<ShaderDesc> & vShaderDescs, 
		const std::vector<ShaderInputAttribute> & vShaderArgs
	);

	void CopyToGfxMem();

public:
	ShaderProgram(
		CustomVertex**	ppVertices,
		unsigned short*	pdwIndices,
		GLuint			nVertices,
		GLuint			nIndices,
		const std::vector<ShaderDesc> & vShaderDescs, 
		const std::vector<ShaderInputAttribute> & vShaderArgs
	);
	~ShaderProgram();

	const bool Initialized() const		{ return m_bInitialized; }

	const GLuint ShaderProgId() const	{ return m_nProgramId; }
	const GLuint VaoId() const			{ return m_nVaoId; }
	const GLuint VboId() const			{ return m_nVboId; }
	const GLuint VboIndexId() const		{ return m_nVboIndexId; }

	bool AssignUniformFloat( const std::string & sName, const float rVal );
	bool AssignUniformMat4( const std::string & sName, const glm::mat4 & matrix );
	bool AssignUniformMat3( const std::string & sName, const glm::mat3 & matrix );
	bool AssignUniformVec4( const std::string & sName, const glm::vec4 & vector );
	bool AssignUniformVec3( const std::string & sName, const glm::vec3 & vector );	
	bool AssignUniformSampler2D( const std::string & sName, const GLuint nTexId );
	void Uninitialize();
};