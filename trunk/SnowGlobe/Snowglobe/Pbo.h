
// Pixel Buffer Object
// Bind Point Info:

// GL_PIXEL_PACK_BUFFER		Target buffer for pixel pack operations such as glReadPixels
//							When a Pbo is attached to this attachment point, any OpenGL operations
//							that read pixels get their data from the Pbo.

// GL_PIXEL_UNPACK_BUFFER	Source buffer for texture update functions such as glTexImage2D or 
//							glTexSubImage2D.
//							When a Pbo is attached to this attachment point, any OpenGL operations
//							that draw pixels put their data into the Pbo.

#pragma once
#include <GXBase.h>
#include <GL\GL.h>

class Pbo : public glex
{
private:	
	bool	m_bInitialized;	
	GLuint	m_nId;
	GLuint	m_nBindPt;			// { GL_PIXEL_PACK_BUFFER || GL_PIXEL_UNPACK_BUFFER }	
	GLuint	m_nPixelDataSize;
	void*	m_pPixelData;
	GLuint	m_nBufferUsage;

private:
	bool Initialize();

public:
	Pbo( GLuint nBindPt = GL_PIXEL_PACK_BUFFER, GLuint nPixelDataSize = 0, void* pPixelData = NULL, GLuint m_nBufferUsage = GL_DYNAMIC_COPY );
	const Pbo & operator=( const Pbo & r );

	~Pbo();

	// accessors
	const GLuint Id()			{ return m_nId; }

	// modifiers
	void Uninitialize();
	bool SetAsReadTarget()	{ if(! m_bInitialized ) { return false; } glBindBuffer( GL_PIXEL_PACK_BUFFER, m_nId ); }
	bool SetAsWriteTarget() { if(! m_bInitialized ) { return false; } glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_nId ); }
};
