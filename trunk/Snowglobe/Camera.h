
// Camera is the view matrix
// Camera is _not_ a graph node
// default copy & assignment should work fine


#pragma once

#include <AntiMatter\precision.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>


class Camera
{
private:
	glm::mat4x4		m_vView;
	glm::vec3		m_vPosition;
	glm::vec3		m_vLookAt;
	glm::vec3		m_vUp;

	float			m_rFOV;
	int				m_nScreenWidth;
	int				m_nScreenHeight;

	float			m_rTranslationSpd;
	float			m_rHorizontalAngle;
	float			m_rVerticalAngle;

public:
	Camera();
	Camera( const glm::vec3 & vPosition, const glm::vec3 & vLookAt, const glm::vec3 & vUp );

	glm::mat4 & V()							{ return m_vView; }	
	const float Angle() const				{ return m_rHorizontalAngle; }
	const float Tilt() const				{ return m_rVerticalAngle; }
	const glm::vec3 & Pos() const			{ return m_vPosition; }
	const glm::vec3 & LookAt() const 		{ return m_vLookAt; }
	const glm::vec3 & Up() const			{ return m_vUp; }

	void Camera::SetPos( const glm::vec3 & r );
	void Update( const float & rSecsDelta );
	void SetScreenSize( const int & x, const int & y ); 
	void SetFOV( const REAL & r );
};
