
#pragma once
#include <GXBase.h>
#include <glex.h>
#include <string>
#include "ShaderTypes.h"

class Shader : public glex
{
private:
	bool			m_bInitialized;
	std::string		m_sShaderFile;
	ShaderType		m_nShaderType;
	GLuint			m_nShaderId;
	GLuint			m_nProgramId;		// This is the ShaderProgram's id

private:	
	bool Initialize();
	bool LoadShader();

public:
	Shader( GLint nProgramId, const std::string & sShaderFile, ShaderType sType );
	Shader( const Shader & r );					// copy
	Shader & operator=( const Shader & r);		// assignment
	~Shader();

	bool Initialized() const		{ return m_bInitialized; }	
	void Uninitialize();	

	GLuint GetShaderId() const		{ return m_nShaderId; }
};