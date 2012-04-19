
#include "stdafx.h"

#include "EffectMgr.h"
#include "Effect.h"

#include "ShaderTypes.h"
#include "ShaderDesc.h"
#include "ShaderInputAttribute.h"
#include "Shader.h"
#include "VertexClass.h"

#include "slowglobe-cfg.h"

#include <AntiMatter\AppLog.h>

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <ticpp.h>


EffectMgr::EffectMgr()  : 
	m_bInitialized	( false ),
	m_sConfigFile	( g_Cfg.EffectMgrCfg() )
{
	m_bInitialized = Initialize();
}
EffectMgr::EffectMgr( const std::string & sConfigFile ) : 
	m_bInitialized	( false ),
	m_sConfigFile	( sConfigFile )
{
	m_bInitialized = Initialize();
}
EffectMgr::~EffectMgr()
{
	if( m_bInitialized )
		Uninitialize();
}


bool EffectMgr::Initialize()
{
	// Check config file exists
	// Parse the config file
	// Create the list of described effects
	
	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using std::ifstream;

	if( m_bInitialized )
		Uninitialize();

	if( ! FileExists(m_sConfigFile) )
		return false;	

	try
	{
		ifstream in;
		in.open( m_sConfigFile, std::ios::in );

		if( in.good() )
			in >> *this;

		in.close();

		m_bInitialized = true;
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s: EffectMgr initialization failed, can't load any shaders!", __FUNCTION__ );
		m_bInitialized = false;
	}

	return m_bInitialized;
}
void EffectMgr::Uninitialize()
{
	// empty the map, deleting any Effect objects off the heap.
	EffectMap::iterator x;

	for( x = m_map.begin(); x != m_map.end(); x ++ )
	{
		Effect* pNext = &(x->second);
		delete pNext;		
	}

	m_map.clear();

	m_bInitialized = false;
}

bool EffectMgr::Add( const std::string & s, Effect * pEffect )
{
	using namespace std;

	if( ! pEffect )
		return false;

	EffectMap::iterator f = m_map.find( s );
	
	if( f != m_map.end() )
		return false;

	m_map.insert( pair<string, Effect &>(s, *pEffect) );

	return true;
}
bool EffectMgr::Find( const std::string & s, Effect & r )
{
	EffectMap::iterator f = m_map.find( s );
	
	if( f == m_map.end() )
		return false;
	
	r = f->second;

	return true;
}
bool EffectMgr::Find( const std::string & s, Effect** pp )
{
	EffectMap::iterator f = m_map.find( s );

	if( f == m_map.end() )
		return false;

	*pp = &(f->second);

	return true;
}

std::ostream & operator << ( std::ostream & out, const EffectMgr & r )
{
	UNREFERENCED_PARAMETER(r);
	return out;
}
std::istream & operator >> ( std::istream & in, EffectMgr & r )
{
	// Create a list of Effect objects from the data in the ticpp::Document	

	using std::string;
	using std::vector;
	using AntiMatter::AppLog;

	ticpp::Document doc;	

	in >> doc;
	
	ticpp::Iterator<ticpp::Element> EffectItr;
	ticpp::Iterator<ticpp::Element> ChildItr;
	ticpp::Iterator<ticpp::Element> AttrItr;	


	for( EffectItr = EffectItr.begin(doc.FirstChildElement()); EffectItr != EffectItr.end(); EffectItr ++ )
	{		
		vector <ShaderDesc>				Descs;
		vector <ShaderInputAttribute>	Attrs;

		string							sEffectName;
		string							sName;
		string							sType;
		string							sTag;

		string							sAttrTag;
		string							sVertexDescType;		// e.g. CustomVertex
		int								nTypeSize = 0;
		string							sFieldName;		

		EffectItr->GetAttribute<string>( string("name"), &sEffectName );

		for( ChildItr = ChildItr.begin(EffectItr.Get()); ChildItr != ChildItr.end(); ChildItr ++ )
		{
			ChildItr->GetValue<string>(&sTag);
			std::transform( sTag.begin(), sTag.end(), sTag.begin(), tolower );
			
			if ( sTag.compare("shader") == 0 )
			{
				ChildItr->GetAttribute<string>( string("name"), &sName );
				ChildItr->GetAttribute<string>( string("type"), &sType );

				ShaderDesc i;
				i.sFileName = sName;

				std::transform( sType.begin(), sType.end(), sType.begin(), tolower );

				if ( sType.compare("vertex") == 0 )
					i.nType = Vertex;
				else if ( sType.compare("fragment") == 0 )
					i.nType = Fragment;
				else if ( sType.compare("geometry") == 0 )
					i.nType = Geometry;
				
				Descs.push_back( i );
			}			
			else if ( sTag.compare("inputattributes") == 0 )
			{				
				for( AttrItr = AttrItr.begin(ChildItr.Get()); AttrItr != AttrItr.end(); AttrItr ++ )
				{
					AttrItr->GetValue<string> (&sAttrTag);
					std::transform( sAttrTag.begin(), sAttrTag.end(), sAttrTag.begin(), tolower );

					if( sAttrTag.compare("vertexdescription") == 0 )
					{
						AttrItr->GetAttribute <string>	( string("type"), &sVertexDescType );
						AttrItr->GetAttribute <int>		( string("stride"), &nTypeSize );	// TODO: lookup type stride via lookup table
					}
					else if ( sAttrTag.compare("attrib") == 0 )
					{
						ShaderInputAttribute j;
					
						AttrItr->GetAttribute <string> ( string("fieldname"),	&j.sFieldName );
						AttrItr->GetAttribute <GLuint> ( string("size"),		&j.nFieldSize );
						AttrItr->GetAttribute <GLuint> ( string("offset"),		&j.nFieldOffset );												

						j.nStride = nTypeSize;

						Attrs.push_back( j );
					}					
				}
			}
		}

		Effect* pE	= new Effect( Descs, Attrs, sEffectName );
		bool bAdded = r.Add( sEffectName, pE );

		if( ! bAdded )
			delete pE;
	}
	
	return in;
}