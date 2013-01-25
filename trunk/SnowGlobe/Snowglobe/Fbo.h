
#pragma once

#include <glex.h>

class Fbo : public glex
{
private:
	GLuint		_nx;
	GLuint		_ny;
	GLint		_nDefaultX;
	GLint		_nDefaultY;
	GLuint		_nFboId;
	GLuint		_nRenderTexId;
	GLuint		_nPixelTexId;
	GLuint		_nDepthBufferId;
	bool		_bInitialized;

private:
	GLuint GenTexture2D( 
		GLuint		x, 
		GLuint		y,
		GLenum		nUnitToActivate = GL_TEXTURE0,
		GLubyte*	pTexData		= NULL,
		GLint		nInternalFmt	= GL_RGBA,
		GLint		nFormat			= GL_RGBA,
		GLenum		nDataFmt		= GL_UNSIGNED_BYTE,
		GLint		nMinFilter		= GL_LINEAR,
		GLint		nMagFilter		= GL_LINEAR )
	{
		// returns the texture handle generated
		GLuint nHandle = 0;

		glGenTextures( 1, &nHandle );
		glActiveTexture( nUnitToActivate );
		glBindTexture( GL_TEXTURE_2D, nHandle );
		
		glTexImage2D( GL_TEXTURE_2D, 0, nInternalFmt, _nx, _ny, 0, nFormat, nDataFmt, pTexData );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nMinFilter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nMagFilter );

		return nHandle;
	}

	void GetCurrentViewportDimensions()
	{
		glGetIntegeri_v( GL_VIEWPORT, 0, &_nDefaultX );
		glGetIntegeri_v( GL_VIEWPORT, 1, &_nDefaultY );
	}

	bool Initialize()
	{	
		GetCurrentViewportDimensions();

		// generate the fbo
		glGenFramebuffers( 1, &_nFboId );
		glBindFramebuffer( GL_FRAMEBUFFER, _nFboId );

		// create the render-to-texture texture
		GLubyte pData[] = {255,255,255,255};
		_nRenderTexId	= GenTexture2D( _nx, _ny, GL_TEXTURE0 );
		_nPixelTexId	= GenTexture2D( 1, 1, GL_TEXTURE1, pData );

		// generate the depth buffer
		glGenRenderbuffers( 1, &_nDepthBufferId );
		glBindRenderbuffer( GL_RENDERBUFFER, _nDepthBufferId );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _nx, _ny );

		// bind texture and depth buffer to the fbo
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _nRenderTexId, 0);
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _nDepthBufferId );

		// set the render target
		GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, drawbuffers );

		// revert to the default render target
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		return true;
	}	
	void Uninitialize()
	{
		_bInitialized = false;

		// delete textures
		glDeleteTextures(1, &_nRenderTexId );
		glDeleteTextures(1, &_nPixelTexId );

		// delete depth buffer
		glDeleteBuffers( 1, &_nDepthBufferId );

		// delete framebuffer
		glDeleteBuffers( 1, &_nFboId );

		_nRenderTexId	= 0;
		_nPixelTexId	= 0;
		_nDepthBufferId = 0;
		_nFboId			= 0;
	}

public:
	Fbo()  : 
		_nx				( 0 ),
		_ny				( 0 ),
		_nDefaultX		( 0 ),
		_nDefaultY		( 0 ),
		_nFboId			( 0 ),
		_nRenderTexId	( 0 ),
		_nPixelTexId	( 0 ),
		_nDepthBufferId	( 0 ),
		_bInitialized	( false )
	{
	}
	Fbo( GLuint x, GLuint y ) : 
		_nx				( x ),
		_ny				( y ),
		_nDefaultX		( 0 ),
		_nDefaultY		( 0 ),
		_nFboId			( 0 ),
		_nRenderTexId	( 0 ),
		_nPixelTexId	( 0 ),
		_nDepthBufferId	( 0 ),
		_bInitialized	( false )
	{
		_bInitialized = Initialize();
	}
	~Fbo()
	{
		Uninitialize();
	}	

	bool Initialized() const	{  return _bInitialized; }	
	GLuint RenderTexId() const	{ return _nRenderTexId; }
	GLuint WhiteTexId() const	{ return _nPixelTexId; }

	bool PreRender()
	{
		if( ! _bInitialized )
			return false;

		// bind to texture's FBO
		glBindFramebuffer( GL_FRAMEBUFFER, _nFboId );
		glViewport( 0, 0, _nx, _ny );

		return true;
	}
	bool PostRender()
	{
		if( ! _bInitialized )
			return false;

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glViewport( 0, 0, _nDefaultX, _nDefaultY );

		return true;
	}
};