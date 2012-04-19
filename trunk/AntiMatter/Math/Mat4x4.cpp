
// Abandoning this class in favour of using glm.
// writing this class is timeconsuming and I'm already behind!

#include "StdAfx.h"
#include "Mat4x4.h"

namespace AntiMatter
{
	Mat4x4::Mat4x4()
	{
	}

	Mat4x4::Mat4x4( const Mat4x4 & r )
	{
		memcpy(&_m, &r, sizeof(mat4_t));
	}

	void Mat4x4::Identity()
	{
		memset(&_m, 0, sizeof(_m));		

		_m.m.m11	= (REAL)1.0;
		_m.m.m22	= (REAL)1.0;
		_m.m.m33	= (REAL)1.0;
		_m.m.m44	= (REAL)1.0;
	}

	void Mat4x4::RX( const REAL & rAngle )
	{

	}

	void Mat4x4::RY( const REAL & rAngle )
	{

	}

	void Mat4x4::RZ( const REAL & rAngle )
	{

	}

	void Mat4x4::Translate( const REAL & x, const REAL & y, const REAL & z )
	{		
	}

	void Mat4x4::Translate( const Vec3 & v )
	{

	}

	void Mat4x4::operator=( const Mat4x4 & r )
	{

	}

	Mat4x4 Mat4x4::operator*( const Mat4x4 & r )
	{
		Mat4x4 res;
		memcpy( &res._m, &_m, sizeof(_m) );

		// res col 1		
		res._m.m.m11 = (_m.m.m11 * res._m.m.m11) + (_m.m.m12 * res._m.m.m21) + (_m.m.m13 * res._m.m.m31) + (_m.m.m14 * res._m.m.m41); // Y
		res._m.m.m21 = (_m.m.m11 * res._m.m.m12) + (_m.m.m12 * res._m.m.m22) + (_m.m.m13 * res._m.m.m32) + (_m.m.m14 * res._m.m.m42); // 
		res._m.m.m31 = (_m.m.m11 * res._m.m.m13) + (_m.m.m32 * res._m.m.m23) + (_m.m.m33 * res._m.m.m33) + (_m.m.m34 * res._m.m.m43);
		res._m.m.m41 = (_m.m.m11 * res._m.m.m14) + (_m.m.m42 * res._m.m.m24) + (_m.m.m43 * res._m.m.m34) + (_m.m.m44 * res._m.m.m44);		

		return res;
	}

	Vec4 Mat4x4::operator[]( const int & n )
	{
		// return row n as Vec4

		if( n < 0 || n > 3 )
		{
			// assert(0);
			return Vec4(0,0,0,0);
		}

		int nRowOffset = n*4;

		return Vec4( _m.a[nRowOffset], _m.a[nRowOffset+1], _m.a[nRowOffset+2], _m.a[nRowOffset+3] );
	}
	Vec4 Mat4x4::operator()( const int & n )
	{
		// return col n as Vec4

		if( n < 0 || n > 3 )
		{
			//assert(0);
			return Vec4(0,0,0,0);
		}

		return Vec4( _m.a[n], _m.a[n+4], _m.a[n+8], _m.a[n+12] );
	}

	void Mat4x4::Transpose()
	{

	}

	Mat4x4	Mat4x4::GetTranspose()
	{
		Mat4x4 result;
		result.Identity();

		return result;
	}
};