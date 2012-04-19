
#include "stdafx.h"
#include "Segment.h"
#include <glm\glm.hpp>
#include "Cylinder.h"

Segment::Segment() :
	m_pParent			( NULL ),
	m_bInitialized		( false ),	
	m_nGeneration		( 0 ),
	m_nNumGenerations	( 0 ),
	m_W					( glm::mat4(1.0) ),
	m_rLength			( 0.0f )		
{
}
Segment::Segment( Segment* pParent, const glm::mat4 & mOrient, const float rLength, const int nNumGenerations ) :
	m_pParent			( pParent ),
	m_bInitialized		( true ),	
	m_nGeneration		( pParent ? pParent->Generation() + 1 : 0 ),
	m_nNumGenerations	( nNumGenerations ),
	m_W					( mOrient ),
	m_rLength			( rLength )
{
}
Segment::~Segment()
{
	Uninitialize();
}

void Segment::Uninitialize()
{
	DeleteChildSegs();

	m_nGeneration	= 0;
	m_rLength		= 0;
	m_pParent		= NULL;
	m_W				= glm::mat4(1);
}
void Segment::AddChildSeg( Segment* pChild )
{
	if( ! pChild )
		return;

	this->m_ChildSegs.push_back(pChild);
}
void Segment::DeleteChildSegs()
{
	for( ChildSegments::iterator n = m_ChildSegs.begin(); n != m_ChildSegs.end(); n ++ )
	{		
		Segment *pNext = *n;
		pNext->DeleteChildSegs();

		delete pNext;
		pNext = NULL;
	}	

	m_ChildSegs.clear();
} 