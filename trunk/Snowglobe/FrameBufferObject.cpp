
#include "stdafx.h"
#include "FrameBufferObject.h"
#include "Texture.h"

#include <gl\GL.h>
#include <AntiMatter\AppLog.h>


FrameBufferObject::FrameBufferObject() :
	m_bInitialized		( false ),
	m_nFboId			( 0 ),
	m_nDepthBuffer		( 0 ),
	m_nWidth			( 0 ),
	m_nHeight			( 0 )
{
}
FrameBufferObject::FrameBufferObject( int nWidth, int nHeight ) :
	m_bInitialized		( false ),
	m_nFboId			( 0 ),
	m_nDepthBuffer		( 0 ),
	m_nWidth			( nWidth ),
	m_nHeight			( nHeight )
{
	m_bInitialized = Initialize();
}
FrameBufferObject::~FrameBufferObject()
{
	Uninitialize();
}

bool FrameBufferObject::Initialize()
{
	if( m_bInitialized )
		return true;

	glex::Load();

	m_Texture = Texture( m_nWidth, m_nHeight );

	if( ! InitiateFBO() )
	{
		Uninitialize();
		return false;
	}

	return true;
}
void FrameBufferObject::Uninitialize()
{
	m_bInitialized = false;

	SetDefaultRenderTarget();

	m_Texture.Uninitialize();

	glDeleteRenderbuffers( 1, &m_nDepthBuffer );
	glDeleteFramebuffers( 1, &m_nFboId );	

	m_nFboId		= 0;
	m_nDepthBuffer	= 0;

}
bool FrameBufferObject::InitiateFBO()
{
	using AntiMatter::AppLog;

	// Generate and bind the frame buffer object
	glGenFramebuffers(1, &m_nFboId );
	glBindFramebuffer( GL_FRAMEBUFFER, m_nFboId );

	// bind the texture to the FBO	
	m_Texture.Select();	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.TextureId(), 0 );
	
	// Create the depth buffer and bind the depth buffer to the FBO
	glGenRenderbuffers( 1, &m_nDepthBuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, m_nDepthBuffer );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_nWidth, m_nHeight );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_nDepthBuffer );

	// check all okay
	GLenum status = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		Uninitialize();
		return false;
	}

	return true;
}

bool FrameBufferObject::SetFBORenderTarget()
{
	if( ! m_bInitialized )
		return false;

	glBindFramebuffer( GL_FRAMEBUFFER, m_nFboId );
	m_Texture.Select();

	// Set the target for the fragment shader outputs
	GLenum eDrawBufs[] = { GL_FRAMEBUFFER, 0 };
	glDrawBuffers(1, eDrawBufs);

	return true;
}
void FrameBufferObject::SetDefaultRenderTarget()
{
	// unbind FBO, revert to the default frame buffer
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}