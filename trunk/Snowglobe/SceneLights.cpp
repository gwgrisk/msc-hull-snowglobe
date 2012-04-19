
#include "stdafx.h"
#include "SceneLights.h"

#include "SceneGraph.h"
#include "Light.h"
#include "slowglobe-cfg.h"

#include <vector>
#include <iostream>
#include <fstream>

#include <AntiMatter\AppLog.h>
#include <AntiMatter\AppException.h>


typedef std::vector<Light*>::iterator	LItr;

SceneLights::SceneLights( SceneGraph * pGraph ) : 
	m_bInitialized	( false ),
	m_sConfigFile	( g_Cfg.SceneLightsCfg() ),
	m_pGraph		( NULL )
{
	if( pGraph )
		m_bInitialized = Initialize( pGraph );
}
SceneLights::~SceneLights()
{
	try
	{
		Clear();
	}
	catch(...)
	{
		// failing silently is the devil :D
		// Parasoft requires an exception safe destructor
		// But curiously, it doesn't care about an exception unsafe ctor!		
	}
}
	
bool SceneLights::Initialize( SceneGraph* pGraph )
{
	if( ! pGraph )
		return false;

	m_pGraph		= pGraph;
	m_bInitialized	= LoadLights();

	if( ! m_bInitialized )
		m_bInitialized = HardcodedLights();

	return m_bInitialized;
}

bool SceneLights::LoadLights()
{
	// load lights config in from disk
	using namespace std;

	bool bRet = false;
	std::ifstream instream;

	instream.open( m_sConfigFile.c_str(), std::ios::in );
	while( instream.good() )
	{
		instream >> *this;
		bRet = true;
	}

	instream.close();

	return bRet;
}
bool SceneLights::HardcodedLights()
{
	using namespace std;

	bool bRet = false;

	// ensure lights list is empty
	Clear();
	
	glm::vec3 La(0.2, 0.2, 0.2);
	glm::vec3 Ld(0.6, 0.6, 0.6);
	glm::vec3 Ls(0.1, 0.1, 0.1);

	glm::vec3 pSun(1.0, 1.0, 1.0);

	// create hard coded lights	
	AddLight( new Light( this->Graph(), this->Graph()->Root(), std::string("light0"),	glm::vec4(100, 400, 400, 1), La, Ld, Ls) );				// light 0
	AddLight( new Light( this->Graph(), this->Graph()->Root(), std::string("light1"),	glm::vec4(100, 400, -400, 1), La, Ld, Ls) );
	AddLight( new Light( this->Graph(), this->Graph()->Root(), std::string("light2"),	glm::vec4(-100, 400, 600, 1), La, Ld, Ls) );
	AddLight( new Light( this->Graph(), this->Graph()->Root(), std::string("light3"),	glm::vec4(-100, 400, -600, 1), La, Ld, Ls) );				// light n
	AddLight( new Light( this->Graph(), this->Graph()->Root(), std::string("sun"),	glm::vec4(0, 200, 0, 1), La, Ld, Ls) );			// the sun

	bRet = true;

	// write lights out to disk
	ofstream out;
	out.open( m_sConfigFile.c_str(), ios::out );

	if( out.good() )
		out << *this;

	return bRet;
}
	
void SceneLights::AddLight( Light* r )
{
	if( ! r )
		return;

	m_vpLights.push_back( r );
}
void SceneLights::Clear()
{	
	for( unsigned int n = 0; n < m_vpLights.size(); n ++ ) 
	{
		Light* pNext = m_vpLights[n];
		delete pNext;
	}

	m_vpLights.clear();
}

// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const SceneLights & r )
{
	// iterate SceneLights, output each light to disk
	const std::vector<Light*> lights = r.Lights();

	for( unsigned int n = 0; n < lights.size(); n ++ )
	{
		Light *pLight = lights[n];
		out << *pLight;
	}

	return out;
}
std::istream & operator >> ( std::istream & in, SceneLights & r )
{	
	// read a light from the stream
	// create a new light object from stream data
	// Add it to the SceneLights vector
	using namespace AntiMatter;
	Light	tmpLight;	

	try
	{		
		in >> tmpLight;
		if( tmpLight.Id().compare("") != 0 )
		{
			tmpLight.SetGraph( r.Graph() );
			tmpLight.SetParent( r.Graph()->Root() );
			r.AddLight( new Light( tmpLight ) );
		}
			
	}
	catch( std::bad_alloc & ba )
	{
		AppLog::Ref().LogMsg("SceneLights::Stream In: bad_alloc while attempting to load scene lights from config file: %s", ba.what() );
	}	
	catch( ... )
	{
		AppLog::Ref().LogMsg("SceneLights::Stream In: Unanticipated exception while attempting to load scene lights from config file:" );
	}	

	return in;
}
