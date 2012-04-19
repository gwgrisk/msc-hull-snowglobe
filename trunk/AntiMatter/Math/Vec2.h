#pragma once
#include "precision.h"
#include "constants.h"

namespace AntiMatter
{
	class Vec2
	{
	public:
		REAL	x, y;

	public:
		Vec2();
		Vec2( const REAL nx, const REAL ny );
		Vec2( const REAL v[2] );

		REAL Len() const;
		REAL LenSquared() const;

		void Normalize();
		static Vec2 Normalize( const Vec2 & r );

		static REAL Dot ( const Vec2 & l, const Vec2 & r );
		REAL Dot ( const Vec2 & r );

		REAL AngleBetweenRad( const Vec2 & r );
		REAL AngleBetweenDeg( const Vec2 & r );

		void operator+=( const Vec2 & r );
		void operator-=( const Vec2 & r );
		void operator*=( const REAL scale );

		Vec2 operator+( const Vec2 & r ) const;
		Vec2 operator-( const Vec2 & r ) const;
		Vec2 operator*( const REAL scale ) const;		
	};
};

