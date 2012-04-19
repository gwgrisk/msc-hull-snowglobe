
#pragma once
#include <glm\glm.hpp>

class Projection
{
private:
	glm::mat4		m_Projection;
	float			m_rFOV;
	float			m_rAspectRatio;
	float			m_rNear;
	float			m_rFar;

private:
	void Update();

public:
	Projection();

	// sets
	void SetProjection( const float & fov, const float & ar, const float & rNear, const float & rFar );
	void FOV( const float & f )								{ m_rFOV = f;				Update(); }
	void AspectRatio( const float & ar )					{ m_rAspectRatio = ar;		Update(); }
	void ClipDist( const float & n, const float & f )		{ m_rNear = n; m_rFar = f;	Update(); }

	// gets
	glm::mat4 & P()											{ return m_Projection; }
	float FOV() const										{ return m_rFOV; }
	float AspectRatio() const								{ return m_rAspectRatio; }
	float NearClipDist() const								{ return m_rNear; }
	float FarClipDist() const								{ return m_rFar; }
};