
#pragma once
#include <string>
#include <GL\GL.h>
#include <gl\GLU.h>

class ShaderInputAttribute
{
public:	
	std::string		sFieldName;
	GLuint			nFieldSize;		// not bytes, number of components
	GLuint			nFieldOffset;
	GLuint			nStride;
};