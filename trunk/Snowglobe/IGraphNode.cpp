
#include "stdafx.h"
#include "IGraphNode.h"
#include <AntiMatter\AppLog.h>

#include <glm\glm.hpp>

IGraphNode::IGraphNode( SceneGraph* pGraph, IGraphNode* pParent, const std::string & id ) :
	m_pGraph	( pGraph ),
	m_pParent	( pParent ),
	m_sId		( id )
{
	m_ChildNodes.resize(0);

	// if parent node exists, set this nodes world matrix = parent's world matrix
	if( m_pParent )
	{
		m_pParent->AddChild(this);
		m_Data.W() = m_pParent->GetNodeData().W();
	}
	else
	{
		// if the node has no parent, it's probably the root node, so set its 
		// world matrix to the identity matrix
		m_Data.W() = glm::mat4(1.0f);
	}
}

HRESULT IGraphNode::Update( const float & rSecsDelta )
{
	if( m_ChildNodes.size() > 0 )
		UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT IGraphNode::UpdateChildren( const float & rSecsDelta )
{
	using namespace std;

	if( m_ChildNodes.size() == 0 )
		return S_OK;

	// iterate child nodes and call Update on each
	for( NItr n = m_ChildNodes.begin(); n != m_ChildNodes.end(); n ++ )
	{
		IGraphNode* pNext = (*n);
		pNext->Update(rSecsDelta);
	}

	return S_OK;
}

HRESULT IGraphNode::DrawItem()
{
	using namespace std;
	
	// iterate child nodes and call Update on each
	for( NItr n = m_ChildNodes.begin(); n != m_ChildNodes.end(); n ++ )
	{
		IGraphNode* pNext = (*n);
		pNext->DrawItem();
	}

	// rendering this node is deliberately ommitted, so the user can override, 
	// call this function, then implement their own DrawItem() on a per object 
	// basis

	return S_OK;
}

void IGraphNode::AddChild( IGraphNode* pChild )
{
	if( ! pChild )
		return;

	pChild->SetParent(this);

	m_ChildNodes.push_back( pChild );
}
