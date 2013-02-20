
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
		GLint		nMagFilter		= GL_LINEAR );

	void GetCurrentViewportDimensions();

	bool Initialize();
	void Uninitialize();

public:
	Fbo();
	Fbo( GLuint x, GLuint y );
	~Fbo();

	bool Initialized() const	{ return _bInitialized; }	
	GLuint RenderTexId() const	{ return _nRenderTexId; }
	GLuint WhiteTexId() const	{ return _nPixelTexId; }

	bool PreRender();
	bool PostRender();
};