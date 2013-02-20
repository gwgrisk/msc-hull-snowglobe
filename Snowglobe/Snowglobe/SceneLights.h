
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Light.h"

class SceneGraph;


class SceneLights
{
public:
	enum LightsState{ sun, spots };

private:
	std::vector<Light *>	m_vpLights;
	bool					m_bInitialized;
	std::string				m_sConfigFile;
	SceneGraph *			m_pGraph;
	LightsState				_lights;

private:	
	bool LoadLights();
	bool HardcodedLights();

public:
	SceneLights( SceneGraph* pGraph = NULL );
	~SceneLights();

	bool Initialize( SceneGraph* pGraph );

	const bool Initialized() const					{ return m_bInitialized; }
	const std::vector<Light*> & Lights() const		{ return m_vpLights; }

	SceneGraph * Graph()							{ return m_pGraph; }

	void AddLight( Light * r );
	void Clear();

	const LightsState GetLightsState() const		{ return _lights; }
	void SetLightsState( const LightsState & e )	{ _lights = e; }
};

// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const SceneLights & r );
std::istream & operator >> ( std::istream & in, SceneLights & r );
