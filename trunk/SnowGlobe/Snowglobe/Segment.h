
#pragma once 
#include <glm\glm.hpp>
#include <AntiMatter\precision.h>
#include <vector>

const real g_kInitialSegLength = 20.0f;


class Segment
{
public:
	typedef std::vector<Segment*> ChildSegments;

private:
	glm::vec3		_vPosition;				// base position
	glm::vec3		_vTipPosition;			// tip position
	glm::vec3		_vOrientation;			// normalized direction
	glm::mat4		_mOrientation;
	glm::vec3		_vPosDelta;
	glm::vec3		_vDeltaOrientation;
	
	int				_nGeneration;
	real			_rLength;
	real			_rScale;

	Segment*		_pParent;
	ChildSegments	_ChildSegments;	

private:
	Segment( const Segment & );					// disallow copy & assignment
	Segment & operator=( const Segment & );

private:
	void Initialize();
	void Update( const real rDeltaSecs );

public:
	// public interface
	Segment();
	Segment( Segment* pParent, const glm::vec3 & vDelta );
	~Segment();

	void AddChild( Segment* pSeg );
	void DeleteChildren();

	// inline accessors
	const Segment* Parent() const 				{ return _pParent; }
	ChildSegments & ChildSegs()					{ return _ChildSegments; }	
	const int Generation()						{ return _nGeneration; }

	const glm::vec3 Pos() const 				{ return _vPosition; }
	const glm::vec3 TipPos() const				{ return _vTipPosition; }
	const glm::vec3 & Orientation() const		{ return _vOrientation; }
	const glm::mat4 & OrientationMtx() const	{ return _mOrientation; }
	const real Length() const 					{ return _rLength; }

	const glm::vec3 PosDelta() const			{ return _vPosDelta; }			
	const glm::vec3 OrientDelta() const			{ return _vDeltaOrientation; }	
	

	void Pos( const glm::vec3 & r ) 			{ _vPosition			= r; }
	void TipPos( const glm::vec3 & r )			{ _vTipPosition			= r; }
	void Orientation( const glm::vec3 & r )		{ _vOrientation			= r; }
	void OrientationMtx( const glm::mat4 & r)	{ _mOrientation		= r; }
	void Length( const real r ) 				{ _rLength				= r; }
	const real Scale() const					{ return _rScale; }

	void PosDelta( const glm::vec3 & r )		{ _vPosDelta			= r; }	
	void OrientDelta( const glm::vec3 & r )		{ _vDeltaOrientation	= r; }
	
	
};