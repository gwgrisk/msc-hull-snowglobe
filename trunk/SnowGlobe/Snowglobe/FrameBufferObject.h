
#pragma once

#include <glex.h>
#include "Texture.h"

class FrameBufferObject : public glex
{
private:
	bool		m_bInitialized;
	
	GLuint		m_nFboId;
	Texture		m_Texture;
	GLuint		m_nDepthBuffer;

	int			m_nWidth;
	int			m_nHeight;

private:
	bool Initialize();
	void Uninitialize();

	bool InitiateFBO();

public:
	FrameBufferObject();
	FrameBufferObject( int nWidth, int nHeight );
	~FrameBufferObject();

	Texture & GetTexture()		{ return m_Texture; }

	bool SetFBORenderTarget();
	void SetDefaultRenderTarget();
};
