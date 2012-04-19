
#pragma once

#include <map>
#include <string>
#include <glm\glm.hpp>
#include <AntiMatter\AppLog.h>
#include "IGraphNode.h"
#include "Camera.h"
#include "Projection.h"
#include "SceneLights.h"

typedef std::map<std::string, IGraphNode*>	ActorMap;
typedef std::pair<std::string, IGraphNode*>	ActorIdPair;
typedef ActorMap::iterator					AItr;


class SceneGraph
{
private:
	class RootNode : public IGraphNode
	{
	public:
		RootNode() :
			IGraphNode( NULL, NULL, std::string("root") )
		{
		}
		HRESULT Update( const float & rSecsDelta )
		{
			IGraphNode::UpdateChildren( rSecsDelta );
			return S_OK;
		}
		HRESULT PreRender()		{ return S_OK; }
		HRESULT Render()		{ return S_OK; }
		HRESULT PostRender()	{ return S_OK; }
	};
		
private:
	Camera*			m_pCamera;
	Projection*		m_pProjection;
	RootNode		m_RootNode;
	ActorMap		m_map;
	SceneLights		m_lights;

	float			m_rYRotationStep;		
	bool			m_bInitialized;

	bool InitializeGraph();
	void DestroyGraph();

private:
	SceneGraph();										// disallow default ctor
	SceneGraph( const SceneGraph & r );					// disallow copy
	SceneGraph & operator=( const SceneGraph & r );		// disallow assignment
		
public:	
	SceneGraph( Camera* pCam, Projection* pProj );	
	~SceneGraph();

	void SetCamera( Camera* p )			{ m_pCamera		= p; }
	void SetProjection( Projection* p )	{ m_pProjection = p; }

	IGraphNode* GetGraphNodeByName( const std::string & sNodeName );

	Camera & Cam()						{ return *m_pCamera; }
	Projection & Proj()					{ return *m_pProjection; }

	IGraphNode* Root()					{ return &m_RootNode; }

	SceneLights & Lights()				{ return m_lights; }

	void Update( float rSecsDelta );
	void Render();
};
