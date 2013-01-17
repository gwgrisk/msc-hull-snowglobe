
#include "stdafx.h"
#include "segment.h"
#include <vector>
#include <algorithm>
#include <glm\glm.hpp>

Segment::Segment() :
	_pParent			(nullptr),
	_vDeltaOrientation	(glm::vec3(0,0,0)),
	_rLength			(g_kInitialSegLength),
	_nGeneration		(0)	
{
}
Segment::Segment(
	Segment* 			pParent, 
	const glm::vec3 & 	vDelta
) : 
	_pParent			(pParent),
	_vDeltaOrientation	(vDelta),
	_rLength			(g_kInitialSegLength),
	_rScale				(1.0f)
{
	Initialize();
}

Segment::~Segment()
{
	DeleteChildren();
}

void Segment::Initialize()
{	
	// initialize position, orientation, length and generation of the segment
	if( _pParent )
	{
		_vPosition			= _pParent->Pos() + (_pParent->Orientation() * _pParent->Length() );
		_vOrientation		= glm::normalize( _pParent->Orientation() + _vDeltaOrientation );
		_vDeltaOrientation	= _vDeltaOrientation;
		_nGeneration		= _pParent->Generation() + 1;
				
		// compute segment length based on segment generation			
		switch( _nGeneration )
		{
			case 0:  _rScale = 1.0f;  break;
			case 1:  _rScale = 0.75f; break;
			case 2:  _rScale = 0.60f;  break;
			case 3:  _rScale = 0.55f;  break;
			default: _rScale = 0.30f; break;
		}
		
		_rLength = _nGeneration > 0 ? (_rScale * _pParent->Length()) : (g_kInitialSegLength) ;
	}
	else
	{
		_vPosition 		= glm::vec3(0,0,0);
		_vOrientation 	= glm::vec3(0,1,0);
		_rLength		= g_kInitialSegLength;
		_rScale			= 1.0f;
		_nGeneration 	= 0;
	}
}

void Segment::Update( const real rDeltaSecs )
{
	// update position, scale and length of the segment

	if( _pParent )
	{
		_vPosition = _pParent->Pos() + ( _pParent->Orientation() * _pParent->Length() );
		
		// compute segment length based on generation
		// and todo: length of segment is also a function of how much "growth" time has 
		// elapsed.  You'll need to consult the season timeline object for this info		
		switch( _nGeneration )
		{
			case 0:  _rScale = 1.0f;  break;
			case 1:  _rScale = 0.75f; break;
			case 2:  _rScale = 0.60f;  break;
			case 3:  _rScale = 0.55f;  break;
			default: _rScale = 0.30f; break;
		}
		
		_rLength = _nGeneration > 0 ? (_rScale * _pParent->Length()) : (g_kInitialSegLength) ;
	}
}

void Segment::AddChild( Segment* pSeg )
{
	_ChildSegments.push_back( pSeg );
}
void Segment::DeleteChildren()
{
	// recursively delete all elements of 
	// the children of this segment

	std::for_each(
		_ChildSegments.begin(), _ChildSegments.end(),
		[]( Segment* p )
		{
			p->DeleteChildren();
		}
	);

	_ChildSegments.erase(
		_ChildSegments.begin(), _ChildSegments.end()
	);
}