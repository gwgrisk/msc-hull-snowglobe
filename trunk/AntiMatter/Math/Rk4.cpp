
#include "stdafx.h"
#include "Rk4.h"


namespace AntiMatter
{
	REAL Rk4::Integrate( MotionFunc f, const REAL dt, const REAL t0, const REAL x0 )
	{
		// assumes that only one timestep is required
		// returns x0 + dx

		REAL tn		= t0;
		REAL xn		= x0;
		REAL rOneSixth = 1.0f / 6.0f;
		REAL k1;
		REAL k2;
		REAL k3;
		REAL k4;
		REAL dx;

		k1		= f( tn, xn );
		k2		= f( tn + (0.5f*dt), xn + (0.5f*k1) );
		k3		= f( tn + (0.5f*dt), xn + (0.5f*k2) );
		k4		= f( tn + dt, xn + k3 );
		dx		= (rOneSixth * ( k1 + (2.0f*k2) + (2.0f*k3) + k4 )) * dt;		

		return x0 + dx;
	}
	void Rk4::IntegrateSteps( std::vector<REAL> & vResults, MotionFunc f, const REAL dt, const REAL t0, const REAL tn, const REAL x0 )
	{	
		// Populates vResults with results of integration over x0..xn with step dt

		//REAL tn		= t0;
		REAL xn		= x0;
		REAL rOneSixth = 1.0f / 6.0f;
		REAL k1;
		REAL k2;
		REAL k3;
		REAL k4;
		REAL dx;

		vResults.clear();
		vResults.reserve( (int)((tn - t0)/dt) );
		vResults.push_back( x0 );

		for( float i = t0; i < tn; i += dt )
		{
			k1		= f( tn, xn );
			k2		= f( tn + (0.5f*dt), xn + (0.5f*k1) );
			k3		= f( tn + (0.5f*dt), xn + (0.5f*k2) );
			k4		= f( tn + dt, xn + k3 );
			dx		= (rOneSixth * ( k1 + (2.0f*k2) + (2.0f*k3) + k4 )) * dt;

			xn += dx;
		}
	}
};