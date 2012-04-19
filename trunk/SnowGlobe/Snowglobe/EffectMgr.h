
#pragma once

#include <GXBase.h>
#include <AntiMatter\Singleton.h>
#include "Effect.h"

#include <map>
#include <string>
#include <fstream>

class EffectMgr :	public Singleton <EffectMgr>,
					public glex
{
private:	
	typedef std::map< std::string, Effect & > EffectMap;
	
	bool			m_bInitialized;
	EffectMap		m_map;	
	std::string		m_sConfigFile;

private:
	EffectMgr( const EffectMgr & );					// disallow copy
	EffectMgr & operator=( const EffectMgr & );		// disallow assignment

	bool Initialize();	

public:
	EffectMgr();
	EffectMgr( const std::string & sConfigFile );
	~EffectMgr();
	
	bool Initialized()	{ return m_bInitialized; }
	void Uninitialize();

	bool Add( const std::string & s, Effect* pEffect );
	bool Find( const std::string & s, Effect & r );
	bool Find( const std::string & s, Effect** pp );
};

std::ostream & operator << ( std::ostream & out, const EffectMgr & r );
std::istream & operator >> ( std::istream & in, EffectMgr & r );