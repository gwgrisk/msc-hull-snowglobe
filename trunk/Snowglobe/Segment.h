
// Segment class is used by the Tree.
// The trunk and branches of the tree are "built" from segments.
// Segments are organised into a tree structure. 
// 
// e.g. the root node is the parent and has zero or more child nodes.
// each child node has one parent node, and zero or mode child nodes.
// The LSystem generates these segments, which are then used to draw 
// the tree by using a number of cylinders.

#pragma once

#include <list>
#include <glm\glm.hpp>

class Segment;
typedef std::list<Segment*>		ChildSegments;

class Segment
{
private:
	Segment*			m_pParent;
	ChildSegments		m_ChildSegs;
	
	bool				m_bInitialized;
	int					m_nGeneration;
	int					m_nNumGenerations;

	glm::mat4			m_W;
	float				m_rLength;	
	

private:
	Segment( const Segment & );					// disallow copy & assignment
	Segment & operator=( const Segment & );

private:
	void Uninitialize();

public:
	Segment();
	Segment( Segment* pParent, const glm::mat4 & mOrient, const float rLength, const int nNumGenerations );
	~Segment();

	void AddChildSeg( Segment* pChild );
	void DeleteChildSegs();

	// gets
	Segment* Parent()						{ return m_pParent; }
	ChildSegments & ChildSegs()				{ return m_ChildSegs; }
	const int Generation() const			{ return m_nGeneration; }
	const float Length() const				{ return m_rLength; }
	const glm::mat4 & W() const				{ return m_W; }
	const glm::vec3 Position() const		{ return glm::vec3(m_W[3][0], m_W[3][1], m_W[3][2]); }

	// sets
	void W( const glm::mat4 & mW )			{ m_W = mW; }
	void Position( const glm::vec3 & r )	{ m_W[3][0] = r.x; m_W[3][1] = r.y; m_W[3][2] = r.z; }
	void Length( const float r )			{ m_rLength = r; }
};
