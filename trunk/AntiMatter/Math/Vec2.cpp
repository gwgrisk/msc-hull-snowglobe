
#include "StdAfx.h"
#include "Vec2.h"
#include "constants.h"
#include <math.h>

namespace AntiMatter
{
	Vec2::Vec2() : 
		x	(0.0f),
		y	(0.0f)
	{
	}
	Vec2::Vec2( const REAL nx, const REAL ny ) : 
		x	( nx ),
		y	( ny )
	{
	}
	Vec2::Vec2( const REAL v[2] )
	{
		x = v[0];
		y = v[1];
	}

	REAL Vec2::Len() const
	{
		// Pythagoras theorem gives vector length
		return sqrt((x*x) + (y*y));
	}
	REAL Vec2::LenSquared() const
	{
		// useful in calculations
		return (x*x + y*y);
	}
	
	void Vec2::Normalize()
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
		}
	}
	Vec2 Vec2::Normalize( const Vec2 & r )
	{
		Vec2 result = r;

		result.Normalize();

		return result;
	}

	REAL Vec2::Dot ( const Vec2 & l, const Vec2 & r )
	{
		return ((l.x * r.x) + (l.y * r.y)) ;
	}
	REAL Vec2::Dot ( const Vec2 & r )
	{
		return ((x * r.x) + (y * r.y)) ;
	}

	REAL Vec2::AngleBetweenRad( const Vec2 & r )
	{			
		REAL rDot		= Dot( *this, r );
		REAL rAngle		= (REAL) acos( ( rDot / (Len()*r.Len())) );

		return rAngle;
	}
	REAL Vec2::AngleBetweenDeg( const Vec2 & r )
	{			
		REAL rAngleRad = AngleBetweenRad(r);

		return rAngleRad * ((REAL)180.0f) / g_Pi;
	}
	
	void Vec2::operator+=( const Vec2 & r  )
	{
		this->x += r.x;
		this->y += r.y;
	}
	void Vec2::operator-=( const Vec2 & r )
	{
		this->x -= r.x;
		this->y -= r.y;
	}
	void Vec2::operator*=( const REAL scale )
	{
		this->x *= scale;
		this->y *= scale;		
	}
		
	Vec2 Vec2::operator+( const Vec2 & r ) const
	{
		Vec2 result(*this);

		result += r;

		return result;
	}
	Vec2 Vec2::operator-( const Vec2 & r ) const
	{
		Vec2 result(*this);

		result -= r;

		return result;
	}

	Vec2 Vec2::operator*( const REAL scale ) const
	{
		Vec2 result(*this);

		result *= scale;

		return result;
	}

};