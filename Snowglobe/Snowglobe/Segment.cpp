
#include "stdafx.h"
#include "segment.h"
#include "Tree.h"
#include <glm\glm.hpp>
#include <vector>
#include <algorithm>

static real nXAxisRotation = 15.0f;
static real nZAxisRotation = 30.0f;


Segment::Segment( Tree* pTree ) :	
	_vPosition			(glm::vec3(0,0,0)),
	_vTipPosition		(glm::vec3(0,0,0)),
	_vOrientation		(glm::vec3(0,1,0)),
	_mOrientation		(glm::mat4(1.0f)),
	_mW					(glm::mat4(1.0f)),
	_nGeneration		(0),
	_rLength			(g_kInitialSegLength),	
	_rScale				(1.0f),
	_pParent			(nullptr),
	_pTree				(pTree)
{
}
Segment::Segment(
	Segment* 			pParent,
	Tree*				pTree,
	const float			nXAxisRotationDelta 
) : 
	_vPosition			(glm::vec3(0,0,0)),
	_vTipPosition		(glm::vec3(0,0,0)),
	_vOrientation		(glm::vec3(0,1,0)),
	_mOrientation		(glm::mat4(1.0f)),
	_mW					(glm::mat4(1.0f)),
	_nGeneration		(0),
	_rLength			(g_kInitialSegLength),	
	_rScale				(1.0f),
	_pParent			(pParent),
	_pTree				(pTree)
{
	if( nXAxisRotationDelta )
		nXAxisRotation += nXAxisRotationDelta;

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
		_vPosition			= _pParent->TipPos();
		_nGeneration		= _pParent->Generation() + 1;				
	}
	else
	{
		_vPosition 		= glm::vec3(0,0,0);
		_nGeneration 	= 0;
		_vOrientation 	= glm::vec3(0,1,0);
		_rLength		= g_kInitialSegLength;
		_rScale			= 1.0f;
	}
}
void Segment::Update( const real rDeltaSecs )
{	
	using glm::vec3;
	using glm::vec4;
	using glm::mat3;
	using glm::mat4;
	using glm::translate;
	using glm::rotate;
	using glm::scale;
	using glm::normalize;

	switch( _nGeneration )
	{
		case 0:  _rScale = 1.0f;  break;
		case 1:  _rScale = 0.65f; break;
		case 2:  _rScale = 0.30f;  break;
		case 3:  _rScale = 0.10f;  break;
		default: _rScale = 0.05f; break;
	}

	_rLength = _rScale * _pTree->CylinderLength();

	if( _pParent )
		_vPosition	= _pParent->TipPos();	
	else	
		_vPosition	= vec3( _pTree->m_Data.W()[3] );		


	// m[] = T * R * S	

	int nSegment	= _pParent ? _pParent->ChildSegNum(this) : 0;
	int nSegments	= _pParent ? _pParent->NumChildSegments() : 1;
	real rAngle		= (_nGeneration % 2 == 0 ? 30.0f : 20.0f);
	real rX			= -90.0f + rAngle * (_nGeneration+1);
	real rZ			= rAngle * (nSegment+1);

	// T
	_mW = translate( mat4(1.0f), _vPosition );

	// R
	if( _nGeneration > 0 )
	{
		_mW *= rotate( mat4(1.0f), rX, vec3(1, 0, 0) );
		_mW *= rotate( mat4(1.0f), rZ, vec3(0, 0, 1) );
	}

	// S
	_mW *= scale( mat4(1.0f), vec3(_rScale) );

	// orientation mtx
	_mOrientation = mat4(mat3(_mW));
	
	// orientation vector
	vec4 vO = _mW * vec4(0, 1, 0, 0);
	_vOrientation = normalize(vec3(vO));

	// tip position
	_vTipPosition = _vPosition + ( _vOrientation * _rLength );
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


int Segment::ChildSegNum( Segment * pChildSeg )
{
	// returns segment position in vector, or -1 for not found

	if( ! _ChildSegments.size() )
		return -1;

	int nSegNum = 0;

	ChildSegments::iterator n;
	for( n = _ChildSegments.begin(); n != _ChildSegments.end(); ++ n )
	{
		if ( (*n) == pChildSeg )
			break;

		++ nSegNum;
	}

	if( n != _ChildSegments.end() )
		return nSegNum;
	else
		return -1;
}	