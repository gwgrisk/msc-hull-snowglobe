
// goals of the scene graph
//
// 1.  manage the world matrix for each node
// 2.  traversal every frame, visit every node and render

#pragma once

#include <list>
#include <string>
#include <glm\glm.hpp>
#include <Windows.h>
#include <AntiMatter\precision.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

class SceneGraph;

class NodeData
{
private:
	glm::mat4		m_Stack;		// rootW * parentW * parentW ...
	glm::mat4		m_MVP;			// MVP holds the MVP for the current object for the current frame (e.g. recomputed each frame)
	glm::mat4		m_World;		// World matrix ( holds	transforms to move model to world space )

public:
	NodeData() :
		m_Stack ( glm::mat4(1.0) ),
		m_MVP	( glm::mat4(1.0) ),
		m_World	( glm::mat4(1.0) )
	{
	}

	glm::mat4 & Stack()	{ return m_Stack; }
	glm::mat4 & MVP()	{ return m_MVP; }
	glm::mat4 & W()		{ return m_World; }	
};

class IGraphNode
{
protected:
	std::string					m_sId;
	NodeData					m_Data;	

	SceneGraph*					m_pGraph;
	IGraphNode*					m_pParent;
	std::list<IGraphNode*>		m_ChildNodes;

protected:
	virtual HRESULT UpdateChildren( const float & rSecsDelta );

public:
	IGraphNode( SceneGraph* pGraph = NULL, IGraphNode* pParent = NULL, const std::string & id = std::string("") );

	// IGraphNode
	virtual HRESULT Update( const float & rSecsDelta );
	virtual HRESULT PreRender() =0;
	virtual HRESULT Render() =0;
	virtual HRESULT PostRender() =0;
	virtual HRESULT DrawItem();
	
	void AddChild( IGraphNode* pChild );

	const std::string & Id() const			{ return m_sId; }	
	IGraphNode* Parent()					{ return m_pParent; }
	NodeData & GetNodeData()				{ return m_Data; } 
	SceneGraph* Graph()						{ return m_pGraph; }
	void SetId( const std::string & s )		{ m_sId = s; }
	void SetParent( IGraphNode* pParent )	{ m_pParent = pParent; }
};

typedef std::list<IGraphNode*>::iterator NItr;