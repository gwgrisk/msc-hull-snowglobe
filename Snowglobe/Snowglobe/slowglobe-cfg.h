
#pragma once
#include <string>
#include <AntiMatter\ShellUtils.h>


class ConfigLocations
{
private:
	std::string		m_sAppDir;
	std::string		m_sCfgDir;
	std::string		m_sAssetsDir;
	std::string		m_sShadersDir;

	std::string		m_sSceneLightsCfg;
	std::string		m_sLSystemCfg;
	std::string		m_sTerrainCfg;
	std::string		m_sPondCfg;
	std::string		m_sTreeShadersCfg;
	std::string		m_sHouseCfg;
	std::string		m_sSnowCfg;
	std::string		m_sSnowfallCfg;
	std::string		m_sDriftCfg;
	std::string		m_sSeasonCfg;
	std::string		m_sEffectMgrCfg;

private:
	void Initialize()
	{
		using AntiMatter::Shell::GetModuleFilePath;
		using std::string;

		HRESULT hr = GetModuleFilePath(m_sAppDir);

		if( FAILED(hr) )		
			m_sAppDir = "C:\\SnowGlobe\\";

#ifdef _DEBUG
		// chop one folder off the module file path/
		// e.g. in debug mode, filepath might be c:\work\project\debug\
		// we want c:\work\project\

		int n = m_sAppDir.rfind(string("\\"));

		if( n != string::npos )
			m_sAppDir.erase(n);

		n = m_sAppDir.rfind(string("\\"));

		if( n != string::npos )
			m_sAppDir.erase(n+1);

#endif

		m_sCfgDir			= m_sAppDir + std::string("Cfg\\");
		m_sAssetsDir		= m_sAppDir + std::string("assets\\");
		m_sShadersDir		= m_sAppDir + std::string("Shaders\\");
		
		m_sSceneLightsCfg	= m_sCfgDir + std::string("lights.cfg");
		m_sLSystemCfg		= m_sCfgDir + std::string("lsystem.cfg");
		m_sTerrainCfg		= m_sCfgDir + std::string("terrain.cfg");
		m_sPondCfg			= m_sCfgDir + std::string("pond.cfg");
		m_sTreeShadersCfg	= m_sCfgDir + std::string("treeshaders.cfg");
		m_sHouseCfg			= m_sCfgDir + std::string("house.cfg");
		m_sSnowCfg			= m_sCfgDir + std::string("snowparticles.cfg");
		m_sSnowfallCfg		= m_sCfgDir + std::string("snowfallparticles.cfg");
		m_sDriftCfg			= m_sCfgDir + std::string("drift.cfg");
		m_sSeasonCfg		= m_sCfgDir + std::string("seasons.cfg");
		m_sEffectMgrCfg		= m_sCfgDir + std::string("EffectsMgr.xml");
	}

public:
	ConfigLocations()	
	{
		Initialize();
	}	

	const std::string & ShadersDir() const		{ return m_sShadersDir; }
	const std::string & AssetsDir()	const		{ return m_sAssetsDir; }

	const std::string & SceneLightsCfg() const	{ return m_sSceneLightsCfg; }
	const std::string & LSystemCfg() const		{ return m_sLSystemCfg; }
	const std::string & TerrainCfg() const		{ return m_sTerrainCfg; }
	const std::string & PondCfg() const			{ return m_sPondCfg; }
	const std::string & TreeShadersCfg() const	{ return m_sTreeShadersCfg; }
	const std::string & HouseCfg() const		{ return m_sHouseCfg; }
	const std::string & SnowCfg() const			{ return m_sSnowCfg; }
	const std::string & SnowfallCfg() const		{ return m_sSnowfallCfg; }

	const std::string & DriftCfg() const		{ return m_sDriftCfg; }
	const std::string & SeasonCfg() const		{ return m_sSeasonCfg; }
	const std::string & EffectMgrCfg() const	{ return m_sEffectMgrCfg; }
};

static ConfigLocations g_Cfg;

// Particle Systems { snow, fire, smoke, leaves, rain }

//[] house

//[] pond
//	[] research
//	[] FrameBufferObject
//	[] reflection map
//	[] normal map
//	[] shader

//[] tree
//		[x] l-system
//		[x] cylinder
//		[x] treeshaderz
//		[??!] scale & draw each cylinder
//		[x] wireframe
//		[] flat
//		[] smooth
//		[] SmoothTextured
//		[] BumpTextured

//[] drift

//[] Particle Systems

//[] lightning
//[] seasons
//[] keyboard input
//[] weather systems
//[] audio (if you've got time)
