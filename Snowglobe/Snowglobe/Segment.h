
#pragma once 
#include <glm\glm.hpp>
#include <AntiMatter\precision.h>
#include <vector>

const real g_kInitialSegLength = 20.0f;

class Tree;

class Segment
{
public:
	typedef std::vector<Segment*> ChildSegments;

private:
	glm::vec3		_vPosition;				// base position
	glm::vec3		_vTipPosition;			// tip position
	glm::vec3		_vOrientation;			// normalized direction
	glm::mat4		_mOrientation;
	glm::mat4		_mW;
	
	int				_nGeneration;
	real			_rLength;
	real			_rScale;

	Segment*		_pParent;
	ChildSegments	_ChildSegments;	
	Tree*			_pTree;
private:
	Segment( const Segment & );					// disallow copy & assignment
	Segment & operator=( const Segment & );

private:
	void Initialize();

public:
	// public interface
	Segment( Tree* pTree );
	Segment( Segment* pParent, Tree* pTree, const float nXAxisRotationDelta = 0.0f );
	~Segment();

	void Update( const real rDeltaSecs );

	void AddChild( Segment* pSeg );
	void DeleteChildren();

	int ChildSegNum( Segment * pChildSeg );
	

	// inline accessors		
	const glm::vec3 Pos() const 				{ return _vPosition; }
	const glm::vec3 TipPos() const				{ return _vTipPosition; }
	const glm::vec3 & Orientation() const		{ return _vOrientation; }
	const glm::mat4 & OrientationMtx() const	{ return _mOrientation; }
	const glm::mat4 & mW() const				{ return _mW; }
	
	const int Generation()						{ return _nGeneration; }
	const real Length() const 					{ return _rLength; }
	const real Scale() const					{ return _rScale; }	
	const Segment* Parent() const 				{ return _pParent; }
	ChildSegments & ChildSegs()					{ return _ChildSegments; }
	int NumChildSegments()						{ return _ChildSegments.size(); }

	void Pos( const glm::vec3 & r ) 			{ _vPosition			= r; }
	void TipPos( const glm::vec3 & r )			{ _vTipPosition			= r; }
	void Orientation( const glm::vec3 & r )		{ _vOrientation			= r; }
	void OrientationMtx( const glm::mat4 & r)	{ _mOrientation			= r; }
	void mW( const glm::mat4 & r )				{ _mW					= r; }
	void Length( const real r ) 				{ _rLength				= r; }	
};