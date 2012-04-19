
#include "StdAfx.h"
#include "Vec3.h"
#include <math.h>

namespace AntiMatter
{
	Vec3::Vec3() : 
		x	(0.0f),
		y	(0.0f),
		z	(0.0f)
	{
	}
	Vec3::Vec3( const REAL nx, const REAL ny, const REAL nz ): 
		x	( nx ),
		y	( ny ),
		z	( nz )
	{
	}	
	Vec3::Vec3( REAL* pv )
	{
		x = pv[0];
		y = pv[1];
		z = pv[2];
	}

	REAL Vec3::Len() const
	{
		// Pythagoras theorem gives vector length
		return sqrt((x*x) + (y*y) + (z*z));
	}
	REAL Vec3::LenSquared() const
	{
		// useful in calculations
		return (x*x + y*y + z*z);
	}
	
	void Vec3::Normalize()
	{
		// Divide each vector component (xyzw) by the vector length to normalize the vector
		// only if it's not already normalized, 
		// and only if length != 0

		REAL len		= Len();
		REAL rAbsLen	= fabs(len);
		
		if( (rAbsLen > (REAL)1.0f)  && (len != (REAL)0.0f) )
		{
			x = x / len;
			y = y / len;
			z = z / len;			
		}
	}
	Vec3 Vec3::Normalize( const Vec3 & r )
	{
		Vec3 result = r;

		result.Normalize();

		return result;
	}

	REAL Vec3::Dot ( const Vec3 & l, const Vec3 & r )
	{
		return ((l.x * r.x) + (l.y * r.y) + (l.z * r.z));
	}
	REAL Vec3::Dot ( const Vec3 & r )
	{
		return ((x * r.x) + (y * r.y) + (z * r.z)) ;
	}

	REAL Vec3::AngleBetweenRad( const Vec3 & r )
	{			
		REAL rDot		= Dot( *this, r );
		REAL rAngle		= (REAL) acos( ( rDot / (Len() * r.Len())) );

		return rAngle;
	}
	REAL Vec3::AngleBetweenDeg( const Vec3 & r )
	{			
		REAL rAngleRad = AngleBetweenRad(r);

		return rAngleRad * ((REAL)180.0f) / g_Pi;
	}

	void Vec3::operator+=( const Vec3 & r  )
	{
		this->x += r.x;
		this->y += r.y;
		this->z += r.z;
	}
	void Vec3::operator-=( const Vec3 & r )
	{
		this->x -= r.x;
		this->y -= r.y;
		this->z -= r.z;
	}
	void Vec3::operator*=( const REAL scale )
	{
		this->x *= scale;
		this->y *= scale;
		this->z *= scale;		
	}
		
	Vec3 Vec3::operator+( const Vec3 & r ) const
	{
		Vec3 result(*this);

		result += r;

		return result;
	}
	Vec3 Vec3::operator-( const Vec3 & r ) const
	{
		Vec3 result(*this);

		result -= r;

		return result;
	}

	Vec3 Vec3::operator*( const REAL scale ) const
	{
		Vec3 result(*this);

		result *= scale;

		return result;
	}
	Vec3 Vec3::operator*( const Vec3 & r ) const
	{	
		// cross product
		return Vec3( 
			((y * r.z) - (z * r.y)), 
			((z * r.x) - (x * r.z)),  
			((x * r.y) - (y * r.x)) 
		);	
	}

};