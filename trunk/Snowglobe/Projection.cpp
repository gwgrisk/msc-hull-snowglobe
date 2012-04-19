
#include "stdafx.h"
#include "Projection.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>


Projection::Projection() :
	m_rFOV			( 45.0f ),
	m_rAspectRatio	( 4.0f / 3.0f ),
	m_rNear			( 0.1f ),
	m_rFar			( 2000.0f )
{
	Update();
}

void Projection::SetProjection( const float & fov, const float & ar, const float & rNear, const float & rFar )
{
	m_rFOV			= fov;
	m_rAspectRatio	= ar;
	m_rNear			= rNear;
	m_rFar			= rFar;
	Update();
}

void Projection::Update()
{
	m_Projection = glm::perspective( m_rFOV, m_rAspectRatio, m_rNear, m_rFar );
}