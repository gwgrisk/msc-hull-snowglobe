
#include "stdafx.h"
#include "SceneGraph.h"

#include <AntiMatter\AppLog.h>
#include <glm\glm.hpp>

#include "Camera.h"
#include "InputMgr.h"
#include "Projection.h"
#include "SceneLights.h"
#include "Terrain.h"
#include "Globe.h"
#include "Tree.h"
#include "House.h"
#include "Pond.h"
#include "Lake.h"
#include "Snow.h"
#include "Snowfall.h"
#include "slowglobe-cfg.h"


SceneGraph::SceneGraph( Camera* pCam, Projection* pProj ) :
	m_pCamera		( pCam ),
	m_pProjection	( pProj ),
	m_bInitialized	( false ),
	m_RootNode		( RootNode() )
{
	if( ! pCam || ! pProj )
		return;

	m_bInitialized	= InitializeGraph();
	
	if( ! m_bInitialized )
		DestroyGraph();
}
SceneGraph::~SceneGraph()
{
	try
	{
		DestroyGraph();
	}
	catch(...)
	{
	}
}

bool SceneGraph::InitializeGraph()
{
	// Create objects
	// Add the objects into the actor map	
	// Once an object is in the SceneGraph, object destruction is responsibility of the SceneGraph
	using AntiMatter::AppLog;
	using std::string;
	using std::vector;
	using glm::mat4;
	using glm::vec3;
	using glm::translate;
	using glm::rotate;

	// Initialize the SceneLights collection (lights need access to the SceneGraph in order to initialize)
	// because of this, light initialization can't be done through plain old RAII
	if( ! m_lights.Initialize(this) )
	{
		AppLog::Ref().LogMsg("SceneGraph lights failed to initialize");
		return false;
	}


	// add lights to the scene graph
	typedef vector<Light*>::const_iterator CItr;
	const vector<Light*> lights = m_lights.Lights();

	for( CItr n = lights.begin(); n != lights.end(); n ++ )
	{
		Light* pLight = (*n);
		m_map.insert( ActorIdPair( pLight->Id(), pLight ) );
	}

	mat4 tr;
	mat4 mW(1.0);


	// Terrain
	Terrain* pTerrain = new (std::nothrow) Terrain(
		this, 
		&m_RootNode, 
		string("terrain"), 
		g_Cfg.AssetsDir() + string("heightfield4.bmp"),			// height map
		g_Cfg.AssetsDir() + string("grassC.jpg"),				// texture map (1 of n ?)
		g_Cfg.AssetsDir() + string("256-circle-alphamap.bmp"),	// alphamap
		128,128,
		4.0f, 4.0f, 0.1f 
	);

	if( ! pTerrain || !  pTerrain->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed to initialize Terrain");
		return false;
	}

	m_map.insert( ActorIdPair( pTerrain->Id(), pTerrain) );
	tr = translate( mat4(1.0), vec3(0, -275, 0) );	
	pTerrain->GetNodeData().W()	*= tr;	

	// Globe (globe is centered on the origin)
	Globe* pGlobe = new (std::nothrow) Globe(  
		this, 
		pTerrain, 
		std::string("globe"), 
		350.0f, 30, 30, 
		string(""), 
		glm::vec4(1.0, 1.0, 1.0, 0.8) 
	);
	
	if( ! pGlobe || ! pGlobe->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed to intialize the Globe");
		return false;
	}
		
	tr = translate( glm::mat4(1.0), vec3(0, 550, 0) );
	pGlobe->GetNodeData().W() *= tr;
	m_map.insert( ActorIdPair( pGlobe->Id(), pGlobe) );
	
	
	// snow
	Snow* pSnow = new (std::nothrow) Snow(
		this,
		&m_RootNode,
		pGlobe,
		string("snowfall"),
		g_Cfg.SnowCfg()
	);

	if( ! pSnow || ! pSnow->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed to allocate heap for snow");
		return false;
	}
		
	tr = translate( mat4(1.0), vec3(0, 350, 0) );
	pSnow->GetNodeData().W() *= tr;	
	m_map.insert( ActorIdPair( pSnow->Id(), pSnow) );
		
	
	/*
	// Snowfall
	Snowfall* pSnowfall = new (std::nothrow) Snowfall(
		this,
		&m_RootNode,
		string("snowfall"),
		g_Cfg.SnowfallCfg()
	);

	if( ! pSnowfall || ! pSnowfall->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed to allocate heap for snowfall");
		return false;
	}

	tr = translate( mat4(1.0), vec3(0, 350, 0) );
	pSnowfall->GetNodeData().W() *= tr;
	m_map.insert( ActorIdPair( pSnowfall->Id(), pSnowfall) );
	*/
	
	// lake
	Lake* pLake = new (std::nothrow) Lake(
		this,
		pTerrain,
		string("lake"),
		180, 390,
		g_Cfg.AssetsDir() + string("water.jpg"),
		g_Cfg.AssetsDir() + string("water-alphamap.bmp"),
		string("") 
	);

	if( ! pLake || ! pLake->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed to initialized the Lake");
		return false;
	}

	tr = translate( mat4(1.0), vec3(0,344,168) );
	mW = rotate( mW, -90.0f, vec3(0, 1, 0) );
	pLake->GetNodeData().W() *= (tr * mW);
	m_map.insert( ActorIdPair( pLake->Id(), pLake) );
	
	/*
	// pond
	Pond* pPond = new (std::nothrow) Pond(
		this,
		pTerrain, 
		string("pond"),
		180, 390, 
		g_Cfg.AssetsDir() + string("water.jpg"),
		g_Cfg.AssetsDir() + string("water-alphamap.bmp"),
		string("") 
	);

	if( ! pPond || ! pPond->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed to initialized the Pond");
		return false;
	}

	tr = translate( mat4(1.0), vec3(0,344,168) );	
	mW = rotate( mW, -90.0f, vec3(0, 1, 0) );
	pPond->GetNodeData().W() *= (tr * mW);
	m_map.insert( ActorIdPair( pPond->Id(), pPond) );
	*/

	// house
	House* pHouse = new (std::nothrow) House(
		this, 
		pTerrain, 
		string("house"), 
		g_Cfg.HouseCfg()
	);

	if( ! pHouse || ! pHouse->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed initialize the house");
		return false;
	}
			
	tr = translate( mat4(1.0), vec3(20, 412, -125) );	
	pHouse->GetNodeData().W() *= tr;
	m_map.insert( ActorIdPair( pHouse->Id(), pHouse) );


	// Tree
	Tree* pTree = new (std::nothrow) Tree(
		this,
		pTerrain,
		string("Tree"),
		g_Cfg.AssetsDir() + string("bark_1.jpg"),
		g_Cfg.AssetsDir() + string("bark_1_bump.jpg")
	);

	if( ! pTree || ! pTree->Initialized() )
	{
		AppLog::Ref().LogMsg("SceneGraph failed to allocate heap for Tree");
		return false;
	}		
	
	tr = translate( mat4(1.0), vec3(0, 352, 70) );
	pTree->GetNodeData().W() *= tr;
	m_map.insert( ActorIdPair( pTree->Id(), pTree) );
	

	return true;
}
void SceneGraph::DestroyGraph()
{
	// Ensure that updating / rendering is halted
	// Iterate the map and destroy the map contents	
	for ( AItr i = m_map.begin(); i != m_map.end(); i ++ )
	{
		ActorIdPair nextpair = *i;
		IGraphNode* pNext = nextpair.second;

		delete pNext;
		m_map.erase( i );
	}

	m_map.clear();
}

void SceneGraph::Update( float rSecsDelta )
{			
	m_RootNode.Update( rSecsDelta );
}
void SceneGraph::Render()
{		
	m_RootNode.DrawItem();
}	 
	
IGraphNode* SceneGraph::GetGraphNodeByName( const std::string & sNodeName )
{
	IGraphNode*			pFound	= NULL;
	ActorMap::iterator	i		= m_map.find(sNodeName);

	if( i != m_map.end() )	
		pFound = i->second;	

	return pFound;
}