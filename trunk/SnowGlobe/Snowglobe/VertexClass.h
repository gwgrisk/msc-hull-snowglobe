
#pragma once

class VertexClass
{
public:
	VertexClass() {}
	virtual ~VertexClass() = 0;
};


class CustomVertex /*: public VertexClass*/
{
public:
	float	m_Position[3];		// x, y, z		// offset 0, size = 3*sizeof(float)	
	float	m_Normal[3];		// nx, ny, nz;	// offset 3
	float	m_TexCoords[2];		// u, v			// offset 6
	float	m_colour[4];		// r, g, b, a	// offset 8
	float	m_Tangent[3];		// r, g, b  	// offset 12
	float	m_Bitangent[3];		// r, g, b  	// offset 15
};