
#pragma once
#include "precision.h"
#include "constants.h"

namespace AntiMatter
{
	class Vec4
	{
	public:
		REAL	x, y, z, w;

	public:
		Vec4();
		Vec4( const REAL nx, const REAL ny, const REAL nz, const REAL nw );
		Vec4( const REAL v[4] );	

		REAL Len() const;
		REAL LenSquared() const;

		void xyzw(REAL* pXYZW) const { pXYZW[0] = x;  pXYZW[1] = y; pXYZW[1] = z; pXYZW[1] = w; }

		void Normalize();
		static Vec4 Normalize( const Vec4 & r );

		static REAL Dot ( const Vec4 & l, const Vec4 & r );
		REAL Dot ( const Vec4 & r );

		REAL AngleBetweenRad( const Vec4 & r );
		REAL AngleBetweenDeg( const Vec4 & r );

		void operator+=( const Vec4 & r );
		void operator-=( const Vec4 & r );
		void operator*=( const REAL scale );

		Vec4 operator+( const Vec4 & r ) const;
		Vec4 operator-( const Vec4 & r ) const;

		Vec4 operator*( const REAL scale ) const;
		Vec4 operator*( const Vec4 & r ) const;		// cross product
	};
};


