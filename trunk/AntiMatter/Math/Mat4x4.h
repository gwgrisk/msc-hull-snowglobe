
#pragma once
#include "math.h"

namespace AntiMatter
{
	class Mat4x4
	{
	private:		
		typedef union mat4x4_t
		{
			struct
			{
				REAL m11, m12, m13, m14;
				REAL m21, m22, m23, m24;
				REAL m31, m32, m33, m34;
				REAL m41, m42, m43, m44;
			} m;

			REAL a[16];
		} mat4_t;
		
	public:
		mat4_t	_m;	

	public:
		Mat4x4();		
		Mat4x4( const Mat4x4 & r );		

		void	Identity();
		void	RX( const REAL & rAngle );
		void	RY( const REAL & rAngle );
		void	RZ( const REAL & rAngle );

		void	Translate( const REAL & x, const REAL & y, const REAL & z );
		void	Translate( const Vec3 & v );		

		void	operator=( const Mat4x4 & r );
		Mat4x4	operator*( const Mat4x4 & r );
		Vec4	operator[]( const int & n );		// return row n as Vec4
		Vec4	operator()( const int & n );		// return col n as Vec4

		void	Transpose();
		Mat4x4	GetTranspose();

		Mat4x4	Determinant( const Mat4x4 & r );
		Mat4x4	GetDeterminant();		

		Mat4x4	Inverse( const Mat4x4 & r );
		bool	Invert();
	};
};

