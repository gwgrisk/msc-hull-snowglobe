
#include "stdafx.h"
#include "Camera.h"
#include <AntiMatter\precision.h>
#include <AntiMatter\constants.h>
#include <AntiMatter\AppLog.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "InputMgr.h"

Camera::Camera() :
	m_vView				( glm::mat4x4(1) ),
	m_vPosition			( glm::vec3(0,0,50) ),
	m_vLookAt			( glm::vec3(0,0,0) ),
	m_vUp				( glm::vec3(0,1,0) ),
	m_rFOV				( 45.0f ),
	m_nScreenWidth		( 1024 ),
	m_nScreenHeight		( 768 ),
	m_rTranslationSpd	( 450.0f ),
	m_rHorizontalAngle	( AntiMatter::g_Pi ),
	m_rVerticalAngle	( 0.0f )
{
	// view matrix
	m_vView = glm::lookAt( m_vPosition, m_vPosition + m_vLookAt, m_vUp );
}
Camera::Camera( const glm::vec3 & vPosition, const glm::vec3 & vLookAt, const glm::vec3 & vUp ) :
	m_vView				( glm::mat4x4(1) ),
	m_vPosition			( vPosition ),
	m_vLookAt			( vLookAt ),
	m_vUp				( vUp ),
	m_rFOV				( 45.0f ),
	m_nScreenWidth		( 1024 ),
	m_nScreenHeight		( 768 ),
	m_rTranslationSpd	( 450.0f ),
	m_rHorizontalAngle	( AntiMatter::g_Pi ),
	m_rVerticalAngle	( 0.0f )
{	
}

void Camera::SetPos( const glm::vec3 & r )
{
	m_vPosition = r;

	// convert spherical co-ords to a direction vector	
	m_vLookAt = glm::vec3(
		cosf( m_rVerticalAngle ) * sinf( m_rHorizontalAngle ),
		sinf( m_rVerticalAngle ),
		cosf( m_rVerticalAngle ) * cosf( m_rHorizontalAngle )
	);	

	
	// right vector (need this to compute the up vector)
	glm::vec3 vRight = glm::vec3(
		cosf( m_rHorizontalAngle - AntiMatter::g_Pi / 2.0f ),
		0,
		sinf( m_rHorizontalAngle - AntiMatter::g_Pi / 2.0f  )
	);
	
	// up vector
	m_vUp = glm::cross( vRight, m_vLookAt );

	// view matrix
	m_vView = glm::lookAt( m_vPosition, m_vPosition + m_vLookAt, m_vUp );

}
void Camera::Update( const float & rSecsDelta )
{	
	using namespace glm;
	
	// convert spherical co-ords to a direction vector	
	m_vLookAt = vec3(
		cos( m_rVerticalAngle ) * sin( m_rHorizontalAngle ),		
		sin( m_rVerticalAngle ),
		cos( m_rVerticalAngle ) * cos( m_rHorizontalAngle )
	);	
	
	// right vector (need this to compute the up vector)
	vec3 vRight = vec3(
		cos( m_rHorizontalAngle - (AntiMatter::g_Pi / 2.0f) ),
		0,
		sin( m_rHorizontalAngle - (AntiMatter::g_Pi / 2.0f) )
	);		
	
	// up vector
	m_vUp = normalize( cross( m_vLookAt, vRight ) );	

	// position
	float rTranslationSpd = InputMgr::Ref().Keybd().Shift() == KeyData::KeyDown ? m_rTranslationSpd * 2.0f : m_rTranslationSpd;

	if( InputMgr::Ref().Keybd().Up() == KeyData::KeyDown )
		m_vPosition += m_vUp * rSecsDelta * rTranslationSpd;

	if( InputMgr::Ref().Keybd().Down() == KeyData::KeyDown )
		m_vPosition -= m_vUp * rSecsDelta * rTranslationSpd;

	if( InputMgr::Ref().Keybd().Forward() == KeyData::KeyDown )
		m_vPosition += m_vLookAt * rSecsDelta * rTranslationSpd;

	if( InputMgr::Ref().Keybd().Back() == KeyData::KeyDown )
		m_vPosition -= m_vLookAt * rSecsDelta * rTranslationSpd;

	// view matrix
	m_vView = lookAt( m_vPosition, m_vPosition + m_vLookAt, m_vUp );	
	
}

void Camera::SetScreenSize( const int & x, const int & y )
{
	m_nScreenHeight = y;
	m_nScreenWidth	= x;
}
void Camera::SetFOV( const float & r )
{
	m_rFOV = r;
}
