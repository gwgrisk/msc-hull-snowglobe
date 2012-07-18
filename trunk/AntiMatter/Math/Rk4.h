
// Implementation of Runge-Kutta 4th order numerical integration
// used to determine the anti-derivatives (mainly for acceleration, velocity, position)


#pragma once

#include <vector>
#include <functional>
#include "precision.h"
#include "constants.h"


namespace AntiMatter
{
	class Rk4
	{
	public:	
		typedef std::function< float(float,float) > MotionEquation;

		static REAL Integrate( const MotionEquation & f, const REAL dt, const REAL t0, const REAL x0 );
		static void IntegrateSteps( std::vector<REAL> & vResults, const MotionEquation & f, const REAL dt, const REAL t0, const REAL tn, const REAL x0 );
	};
};