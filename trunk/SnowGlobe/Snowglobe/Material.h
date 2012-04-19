
#pragma once

#include <glm\glm.hpp>

class Material
{
private:
	glm::vec3	m_Ka;
	glm::vec3	m_Kd;
	glm::vec3	m_Ks;
	float		m_rShininess;
		
public:	
	Material() :
		m_Ka			(glm::vec3(0.3, 0.3, 0.3)),
		m_Kd			(glm::vec3(0.8, 0.8, 0.8)),
		m_Ks			(glm::vec3(0.0, 0.0, 0.0)),
		m_rShininess	(1.0f)
	{
	}
	Material( const glm::vec3 & Ka, const glm::vec3 & Kd, const glm::vec3 & Ks, const float rShine ) :
		m_Ka			( Ka ),
		m_Kd			( Kd ),
		m_Ks			( Ks ),
		m_rShininess	( rShine )
	{
	}
	
	const glm::vec3 & Ka() const	{ return m_Ka; }
	const glm::vec3 & Kd() const	{ return m_Kd; }
	const glm::vec3 & Ks() const	{ return m_Ks; }
	const float Shininess() const	{ return m_rShininess; }

	void Ka( const glm::vec3 & Ka ) { m_Ka = Ka; }
	void Kd( const glm::vec3 & Kd ) { m_Ka = Kd; }
	void Ks( const glm::vec3 & Ks ) { m_Ka = Ks; }
	void Shininess( const float r ) { m_rShininess = r; }
};

std::ostream & operator << ( std::ostream & out, const Material & r );
std::istream & operator >> ( std::istream & in, Material & r );