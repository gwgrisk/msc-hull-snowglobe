
#include "StdAfx.h"
#include "Vec4.h"
#include <math.h>
#include <atlbase.h>

namespace AntiMatter
{
	Vec4::Vec4() : 
		x	(0.0f),
		y	(0.0f),
		z	(0.0f),
		w	(0.0f)
	{
	}
	Vec4::Vec4( const REAL nx, const REAL ny, const REAL nz, const REAL nw ): 
		x	( nx ),
		y	( ny ),
		z	( nz ),
		w	( nw )
	{
	}
	Vec4::Vec4( const REAL v[4] )
	{
		x = v[0];
		y = v[1];
		z = v[2];
		w = v[3];
	}

	REAL Vec4::Len() const
	{
		// Pythagoras theorem gives vector length
		return sqrt((x*x) + (y*y) + (z*z) + (w*w));
	}
	REAL Vec4::LenSquared() const
	{
		// useful in calculations
		return (x*x + y*y + z*z + w*w);
	}
	
	void Vec4::Normalize()
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
			w = w / len;
		}
	}
	Vec4 Vec4::Normalize( const Vec4 & r )
	{
		Vec4 result = r;

		result.Normalize();

		return result;
	}

	REAL Vec4::Dot ( const Vec4 & l, const Vec4 & r )
	{
		return ((l.x * r.x) + (l.y * r.y) + (l.z * r.z) + (l.w * r.w));
	}
	REAL Vec4::Dot ( const Vec4 & r )
	{
		return ((x * r.x) + (y * r.y) + (z * r.z)) ;
	}

	REAL Vec4::AngleBetweenRad( const Vec4 & r )
	{			
		REAL rDot		= Dot( *this, r );
		REAL rAngle		= (REAL) acos( ( rDot / (Len() * r.Len())) );

		return rAngle;
	}
	REAL Vec4::AngleBetweenDeg( const Vec4 & r )
	{			
		REAL rAngleRad = AngleBetweenRad(r);

		return rAngleRad * ((REAL)180.0f) / g_Pi;
	}

	void Vec4::operator+=( const Vec4 & r  )
	{
		this->x += r.x;
		this->y += r.y;
		this->z += r.z;
		this->w += r.w;
	}
	void Vec4::operator-=( const Vec4 & r )
	{
		this->x -= r.x;
		this->y -= r.y;
		this->z -= r.z;
		this->w -= r.w;
	}
	void Vec4::operator*=( const REAL scale )
	{
		this->x *= scale;
		this->y *= scale;
		this->z *= scale;
		this->w *= scale;
	}
		
	Vec4 Vec4::operator+( const Vec4 & r ) const
	{
		Vec4 result(*this);

		result += r;

		return result;
	}
	Vec4 Vec4::operator-( const Vec4 & r ) const
	{
		Vec4 result(*this);

		result -= r;

		return result;
	}

	Vec4 Vec4::operator*( const REAL scale ) const
	{
		Vec4 result(*this);

		result *= scale;

		return result;
	}

	Vec4 Vec4::operator*( const Vec4 & r ) const 
	{
		ATLASSERT(0);
		// I'm sure this isn't right; this is the formula for Vec3 cross product
		return Vec4( 
			((y * r.z) - (z * r.y)), 
			((z * r.x) - (x * r.z)),  
			((x * r.y) - (y * r.x)),
			1
		);	
	}
};