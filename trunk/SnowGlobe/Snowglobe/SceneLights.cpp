
#include "stdafx.h"
#include "SceneLights.h"

#include "SceneGraph.h"
#include "Light.h"
#include "slowglobe-cfg.h"
#include "Colours.h"

#include <vector>
#include <iostream>
#include <fstream>

#include <AntiMatter\AppLog.h>
#include <AntiMatter\Constants.h>
#include <AntiMatter\AppException.h>

#include <glm\glm.hpp>


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
	// if a lights.cfg file doesn't exist, then this function
	// creates some defaults and creates the cfg file for future
	// use.

	using namespace std;
	using AntiMatter::g_Pi;

	bool bRet = false;

	// ensure lights list is empty
	Clear();	
	
	glm::vec3 vNone(0.0, 0.0, 0.0);	
	glm::vec3 vFull(1.0, 1.0, 1.0);

	float rMultiplier	= 6.0f;

	for( unsigned int n = 0; n < 4; ++ n )
	{
		std::stringstream	ss;
		std::string			sName; 

		ss << "light" << n;
		sName = ss.str();
		
		float rRadius		= 650.0f;
		float rSlices		= 4.0f;
		float rTheta		= ((2.0f * g_Pi) / rSlices);
		float x				= rRadius * -cosf((float)n * rTheta);
		float z				= rRadius * sinf((float)n * rTheta);
		glm::vec3 vColour	= glm::vec3(Colours::g_ColourList[n]);
		SceneGraph* pGraph	= this->Graph();

		AddLight( 
			new Light( 
				pGraph, 
				pGraph->Root(), 
				sName, 
				glm::vec4(x, rRadius, z, 1), 
				vNone, 
				vColour,
				vColour * rMultiplier,
				Light::LightType::SpotLight,
				glm::vec3(0, -1, 0),
				1.0f,
				15.0f
			)
		);
	}

	// sun light
	AddLight( new Light( 
			this->Graph(), 
			this->Graph()->Root(), 
			std::string("sun"),	
			glm::vec4(0.0, 500.0, 0.0, 1.0), 
			vNone, 
			vFull,
			vFull * rMultiplier
		) 
	);
		
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
