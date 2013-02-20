
#include "stdafx.h"
#include "SeasonalTimeline.h"

#include <AntiMatter\AppException.h>
#include <AntiMatter\ShellUtils.h>
#include <fstream>
#include <string>

SeasonalTimeline::SeasonalTimeline() :
	m_bInitialized		( false ),
	m_rSpeed			( 0.0f ),
	m_rPrevSpeed		( 0.0f ),
	m_rSeasonTimeline	( 0.0f ),
	m_rSeasonDuration	( 0.0f )
{
}
SeasonalTimeline::SeasonalTimeline( const SeasonalTimeline & r )  :
	m_bInitialized		( r.m_bInitialized ),
	m_rSpeed			( r.m_rSpeed ),
	m_rPrevSpeed		( r.m_rPrevSpeed ),
	m_rSeasonTimeline	( r.m_rSeasonTimeline ),
	m_rSeasonDuration	( r.m_rSeasonDuration )
{
}
SeasonalTimeline & SeasonalTimeline::operator=( const SeasonalTimeline & r )
{
	if( this != &r )
	{
		m_bInitialized		= r.m_bInitialized;
		m_rSpeed			= r.m_rSpeed;
		m_rPrevSpeed		= r.m_rPrevSpeed;
		m_rSeasonTimeline	= r.m_rSeasonTimeline;
		m_rSeasonDuration	= r.m_rSeasonDuration;
	}

	return *this;
}
SeasonalTimeline::SeasonalTimeline( const std::string & sConfigFile ) :
	m_bInitialized		( false ),
	m_rSpeed			( 0.0f ),
	m_rPrevSpeed		( 0.0f ),
	m_rSeasonTimeline	( 0.0f ),
	m_rSeasonDuration	( 0.0f )
{
	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;	
	using std::ifstream;


	// attempt to load data from config file
	if( ! FileExists( sConfigFile ) )
	{
		AppLog::Ref().LogMsg("%s config file not found", __FUNCTION__ );
		return;
	}
	
	ifstream in;
	in.open( sConfigFile, std::ios::in );

	if( in.good() )
	{
		in >> *this;
		in.close();

		m_bInitialized = true;
	}
	else
	{
		AppLog::Ref().LogMsg("%s config file invalid", __FUNCTION__ );			
	}	
}
SeasonalTimeline::~SeasonalTimeline()
{
}
	
// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const SeasonalTimeline & r )
{
	using AntiMatter::AppLog;

	try
	{
		out << "SeasonTimeline\n";
		out << r.Speed() << " ";
		out << r.PrevSpeed() << " ";
		out << r.SeasonTimeline() << " ";
		out << r.SeasonDuration() << std::endl;
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated (probably disk) exception, or invalid config file", __FUNCTION__ );
	}

	return out;
}
std::istream & operator >> ( std::istream & in, SeasonalTimeline & r )
{
	using AntiMatter::AppLog;

	char sBuffer[16];
	float rNext = 0.0f;

	try
	{
		in >> sBuffer;

		if( strcmp(sBuffer, "SeasonTimeline") != 0 )
			throw;

		in >> rNext; r.Speed(rNext);
		in >> rNext; r.PrevSpeed(rNext);
		in >> rNext; r.SeasonTimeline(rNext);
		in >> rNext; r.SeasonDuration(rNext);
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated (probably disk) exception, or invalid config file", __FUNCTION__ );
	}

	return in;
}