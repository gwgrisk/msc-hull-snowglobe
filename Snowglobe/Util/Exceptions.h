
#pragma once

#include "AppException.h"
#include <string>

template <typename E, typename A>
void Enforce(bool bCondition, A arg)
{
	if( ! bCondition )	
		throw E(arg);	
}