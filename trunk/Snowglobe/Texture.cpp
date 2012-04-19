
#include "stdafx.h"
#include "Texture.h"
#include <string>
#include <AntiMatter\AppLog.h>
#include <AntiMatter\ShellUtils.h>


Texture::Texture() : 
	m_bInitialized	( false ),
	m_nTextureId	( 0 ),
	m_nX			( 0 ),
	m_nY			( 0 ),
	m_nMinFilter	( GL_LINEAR ),
	m_nMagFilter	( GL_LINEAR )
{
	glex::Load();
}
Texture::Texture ( GLuint x, GLuint y, GLuint nMinFilter, GLuint nMagFilter, void* pTextureData ) : 
	m_bInitialized	( false ),
	m_nTextureId	( 0 ),
	m_nX			( x ),
	m_nY			( y ),
	m_nMinFilter	( nMinFilter ),
	m_nMagFilter	( nMagFilter )
{
	glex::Load();
	m_bInitialized = CreateTexture(pTextureData);
}
Texture::Texture( const std::string & sFilename, bool bWrap ) :
	m_bInitialized	( false ),
	m_nTextureId	( 0 ),
	m_nX			( 0 ),
	m_nY			( 0 ),
	m_nMinFilter	( GL_LINEAR ),
	m_nMagFilter	( GL_LINEAR ),
	m_sTextureFile	( sFilename )
{	
	glex::Load();	
	m_bInitialized = LoadToGfxMem( bWrap );
}
Texture & Texture::operator=( const Texture & rhs )
{
	if( this != &rhs )
	{
		this->Uninitialize();

		if( rhs.Initialized() )
		{
			if( rhs.m_sTextureFile.length() > 0 )
			{
				m_sTextureFile = rhs.m_sTextureFile;
				m_bInitialized = LoadToGfxMem();
			}
			else
			{
				m_nX			= rhs.m_nX;
				m_nY			= rhs.m_nY;
				m_nMinFilter	= rhs.m_nMinFilter;
				m_nMagFilter	= rhs.m_nMagFilter;
				m_bInitialized	= CreateTexture(NULL);
			}			
		}
	}

	return *this;
}
Texture::~Texture()
{
	Uninitialize();
}

bool Texture::CreateTexture( void* pTextureData )
{
	// generates an RGBA texture, initialized with pTextureData (normally null)

	if( m_bInitialized )
		return false;

	glGenTextures( 1, &m_nTextureId );
	if( m_nTextureId == 0xffffffff )
		return false;

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_nTextureId );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_nX, m_nY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTextureData );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_nMinFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_nMagFilter );

	glBindTexture( GL_TEXTURE_2D, 0 );

	return true;
}
bool Texture::LoadToGfxMem( bool bWrap )
{	
	using namespace AntiMatter;

	if( m_bInitialized )
		return true;

	if( ! Shell::FileExists( m_sTextureFile ) )
		return false;
	
	gxbase::Image	img;

	if( ! img.Load( m_sTextureFile.c_str() ) )
		return false;

	if( ! img.IsValid() )
		return false;	

	if( (img.Width() % 2 != 0) || (img.Height() % 2 != 0) )
		img.ScaleImagePow2();

	m_nX	= img.Width();
	m_nY	= img.Height();
		
	glGenTextures( 1, &m_nTextureId );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_nTextureId );
	
	// wrap or clamp the texture
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bWrap ? GL_REPEAT : GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bWrap ? GL_REPEAT : GL_CLAMP );

	// build 2d mip maps
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_nMinFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_nMagFilter );

	img.gluBuild2DMipmaps();
	
	img.Free();
	glBindTexture( GL_TEXTURE_2D, 0 );

	return true;
}
void Texture::Uninitialize()
{	
	if( ! m_bInitialized )
		return;

	glDeleteTextures(1, &m_nTextureId );

	m_nTextureId	= 0;
	m_sTextureFile	= "";
	m_nX			= 0;
	m_nY			= 0;

	m_bInitialized	= false;
}