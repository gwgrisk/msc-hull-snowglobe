
#pragma once
#include <gl\GL.h>
#include <GXBase.h>
#include <string>


class Texture : public glex
{
private:
	bool			m_bInitialized;
	GLuint			m_nTextureId;
	std::string		m_sTextureFile;

	GLuint			m_nX;
	GLuint			m_nY;
	GLuint			m_nMinFilter;
	GLuint			m_nMagFilter;

private:
	bool LoadToGfxMem( bool bWrap = true );	
	bool CreateTexture( void* pTextureData );

public:
	Texture();
	Texture( GLuint x, GLuint y, GLuint nMinFilter = GL_LINEAR, GLuint nMagFilter = GL_LINEAR, void* pTextureData = NULL );
	Texture( const std::string & sFilename, bool bWrap = true );
	Texture & operator=(const Texture & rhs );
	~Texture();	

	void Uninitialize();

	const bool		Initialized() const		{ return m_bInitialized; }
	const GLuint	TextureId() const		{ return m_nTextureId; } 
	const GLuint	Width() const			{ return m_nX; }
	const GLuint	Height() const			{ return m_nY; }

	void Select() 							{ glActiveTexture(GL_TEXTURE0); glBindTexture( GL_TEXTURE_2D, m_nTextureId ); }
};