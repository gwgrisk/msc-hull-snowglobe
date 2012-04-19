#pragma once
#include "precision.h"
#include "constants.h"

namespace AntiMatter
{
	class Vec3
	{
	public:
		REAL	x, y, z;

	public:
		Vec3();
		Vec3( const REAL nx, const REAL ny, const REAL nz );		
		Vec3( REAL* pv );		

		REAL Len() const;
		REAL LenSquared() const;

		void Normalize();
		static Vec3 Normalize( const Vec3 & r );

		static REAL Dot ( const Vec3 & l, const Vec3 & r );
		REAL Dot ( const Vec3 & r );

		REAL AngleBetweenRad( const Vec3 & r );
		REAL AngleBetweenDeg( const Vec3 & r );

		void operator+=( const Vec3 & r );
		void operator-=( const Vec3 & r );
		void operator*=( const REAL scale );

		Vec3 operator+( const Vec3 & r ) const;
		Vec3 operator-( const Vec3 & r ) const;

		Vec3 operator*( const REAL scale ) const;		

		Vec3 operator*( const Vec3 & r ) const;
	};
};

