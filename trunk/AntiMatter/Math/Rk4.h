
// Implementation of Runge-Kutta 4th order numerical integration
// used to determine the anti-derivatives (mainly for acceleration, velocity, position)


#pragma once

#include <vector>
#include "precision.h"
#include "constants.h"


namespace AntiMatter
{
	class Rk4
	{
	public:	
		typedef REAL (*MotionFunc)( const REAL t, const REAL x );

		static REAL Integrate( MotionFunc f, const REAL dt, const REAL t0, const REAL x0 );
		static void IntegrateSteps( std::vector<REAL> & vResults, MotionFunc f, const REAL dt, const REAL t0, const REAL tn, const REAL x0 );		
	};
};