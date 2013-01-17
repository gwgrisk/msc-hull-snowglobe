
#pragma once


class VertexClass
{
public:
	virtual ~VertexClass() = 0;
};


class CustomVertex
{
public:
	float	m_Position[3];		// x, y, z		// offset 0, size = 3*sizeof(float)	
	float	m_Normal[3];		// nx, ny, nz;	// offset 3
	float	m_TexCoords[2];		// u, v			// offset 6
	float	m_Colour[4];		// r, g, b, a	// offset 8
	float	m_Tangent[3];		// r, g, b  	// offset 12
	float	m_Bitangent[3];		// r, g, b  	// offset 15

	CustomVertex()
	{
	}

	CustomVertex( const CustomVertex & r )
	{
		memcpy(this, &r, sizeof(CustomVertex));
	}

	CustomVertex & operator= ( const CustomVertex & r )
	{
		if( &r != this )		
			memcpy(this, &r, sizeof(CustomVertex));

		return *this;
	}
};

/*
class CustomVertex		// total size = 80 bytes (given that float is 4 bytes)
{
public:
	union // Position		offset 0, size = sizeof(float)*3
	{
		struct	{ float x; float y; float z;  };
		float	m_Position[3];		
	};

	union // Normal			offset 3, size = sizeof(float)*3
	{
		struct		{ float nx; float ny; float nz; };
		float		m_Normal[3];		
	};

	union // TexCoords		offset 6, size = sizeof(float)*2
	{
		struct{ float u; float v; };
		float m_TexCoords[2];
	};

	union // Colour			offset 8, size = sizeof(float)*4
	{
		struct{ float r; float g; float b; float a; };
		float m_Colour[4];
	};

	union // Tangent		offset 12, size = sizeof(float)*3
	{
		struct{ float tx; float ty; float tz; };
		float m_Tangent[3];
	};

	union // Bitangent		offset 15, size = sizeof(float) * 3
	{
		struct{ float bitx; float bity; float bitz; };
		float m_BiTangent[3];
	};		

	unsigned char padding[8];	// align to 16 byte boundary
};
*/