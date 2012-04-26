
#pragma once

#include <glm\glm.hpp>
#include <string>

class Material
{
private:	
	glm::vec3		m_Ka;
	glm::vec3		m_Kd;
	glm::vec3		m_Ks;
	float			m_rShininess;
	std::string		m_sName;

	unsigned int	m_Ns;			// Added these since they're added by 3ds max wavefront obj exporter
	unsigned int	m_d;
	unsigned int	m_Tr;
	glm::vec3		m_Tf;
	unsigned int	m_Illum;
		
public:	
	Material() :
		m_Ka			(glm::vec3(0.8, 0.8, 0.8)),
		m_Kd			(glm::vec3(0.8, 0.8, 0.8)),
		m_Ks			(glm::vec3(0.2, 0.2, 0.2)),
		m_rShininess	(1.0f),
		m_Ns			( 32 ),
		m_d				( 1 ),
		m_Tr			( 0 ),
		m_Tf			( glm::vec3(1,1,1) ),
		m_Illum			( 2 )
	{
	}
	Material( const std::string & sName, const glm::vec3 & Ka, const glm::vec3 & Kd, const glm::vec3 & Ks, const float rShine ) :		
		m_sName			( sName ),
		m_Ka			( Ka ),
		m_Kd			( Kd ),
		m_Ks			( Ks ),
		m_rShininess	( rShine ),
		m_Ns			( 32 ),
		m_d				( 1 ),
		m_Tr			( 0 ),
		m_Tf			( glm::vec3(1,1,1) ),
		m_Illum			( 2 )
	{
	}
	Material( const std::string & sName, const glm::vec3 & Ka, const glm::vec3 & Kd, const glm::vec3 & Ks, const float rShine,
			  const unsigned int ns, const unsigned int d, const unsigned int tr, const glm::vec3 & tf, const unsigned int illum ) :		
		m_sName			( sName ),
		m_Ka			( Ka ),
		m_Kd			( Kd ),
		m_Ks			( Ks ),
		m_rShininess	( rShine ),
		m_Ns			( ns ),
		m_d				( d ),
		m_Tr			( tr ),
		m_Tf			( tf ),
		m_Illum			( illum )
	{
	}
	
	const std::string & Name() const	{ return m_sName; } 
	const glm::vec3 & Ka() const		{ return m_Ka; }
	const glm::vec3 & Kd() const		{ return m_Kd; }
	const glm::vec3 & Ks() const		{ return m_Ks; }
	const float Shininess() const		{ return m_rShininess; }

	unsigned int Ns() const				{ return m_Ns; }
	unsigned int D() const				{ return m_d; } 
	unsigned int Tr() const				{ return m_Tr; }
	const glm::vec3 & Tf() const		{ return m_Tf; }
	unsigned int Illum() const			{ return m_Illum; }

	void Name( const std::string & s )	{ m_sName		= s; }
	void Ka( const glm::vec3 & Ka )		{ m_Ka			= Ka; }
	void Kd( const glm::vec3 & Kd )		{ m_Ka			= Kd; }
	void Ks( const glm::vec3 & Ks )		{ m_Ka			= Ks; }
	void Shininess( const float r )		{ m_rShininess	= r; }
	
	void Ns( const unsigned int & n )	{ m_Ns		= n; }
	void D( const unsigned int & n )	{ m_d		= n; }
	void Tr( const unsigned int & n )	{ m_Tr		= n; }
	void Tf( const glm::vec3 & n )		{ m_Tf		= n; }
	void Illum( const unsigned int & n ){ m_Illum	= n; }
};

std::ostream & operator << ( std::ostream & out, const Material & r );
std::istream & operator >> ( std::istream & in, Material & r );