
#include "stdafx.h"

#include "LSystem.h"
#include "Segment.h"
#include "slowglobe-cfg.h"

#include <AntiMatter\AppLog.h>
#include <AntiMatter\ShellUtils.h>
#include <comdef.h>

#include <string>
#include <vector>

#include <iostream>
#include <fstream>

#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>

using std::string;
using std::vector;
using glm::vec3;


LSystem::LSystem() :
	m_bInitialized		( false ),
	m_nMaxGenerations	( 0 ),
	m_pRootNode			( NULL ),
	m_rAngle			( 0.0f ),
	m_rInitialSegLength	( 0.0f ),
	m_vInitialPosition	( glm::vec3(0.0f) ),
	m_vNextDirection	( glm::vec3(0.0f) ),
	_pTree				( nullptr )
{
}
LSystem::LSystem( Tree* pTree, const std::string & sCfgFile ) :
	m_bInitialized		( false ),
	m_nMaxGenerations	( 0 ),
	m_pRootNode			( NULL ),
	m_rAngle			( 0.0f ),
	m_rInitialSegLength	( 0.0f ),
	m_vInitialPosition	( glm::vec3(0.0f) ),
	m_vNextDirection	( glm::vec3(0.0f) ),
	_pTree				( pTree )
{
	// read in ruleset from disk, then Initialize
	using AntiMatter::Shell::FileExists;
	using std::ifstream;

	if( FileExists(sCfgFile) )
	{
		ifstream in;
		in.open( sCfgFile, std::ios::in );

		if( in.good() )
			in >> *this;

		in.close();

		// reading in from file calls the copy ctor
		// which means we don't need to initialize here
		// this->Initialize();
	}
}
LSystem::LSystem( Tree* pTree, const int & nMaxGenerations, const float rAngle, const float rInitSegLen, const glm::vec3 & vInitialPos, const vector<string> & sRuleset ) :
	m_bInitialized		( false ),
	m_nMaxGenerations	( nMaxGenerations ),
	m_pRootNode			( NULL ),
	m_rAngle			( rAngle ),
	m_rInitialSegLength	( rInitSegLen ),
	m_vInitialPosition	( vInitialPos ),
	m_vNextDirection	( glm::vec3(0.0f) ),
	_pTree				( pTree )
{		
	this->Rules( sRuleset );

	if( nMaxGenerations < 2 || m_rAngle < 0 || m_rInitialSegLength < 1.0f || sRuleset.size() == 0 )
	{
		AntiMatter::AppLog::Ref().LogMsg("Invalid parameters specified to LSystem constructor");
		return;
	}

	m_bInitialized = Initialize();
}
LSystem::LSystem( const LSystem & r )  :
	m_bInitialized		( false ),
	m_nMaxGenerations	( r.m_nMaxGenerations ),
	m_pRootNode			( NULL ),
	m_rAngle			( r.m_rAngle ),
	m_rInitialSegLength	( r.m_rInitialSegLength ),
	m_vInitialPosition	( r.m_vInitialPosition ),
	m_vNextDirection	( r.m_vNextDirection ),
	_pTree				( r._pTree )
{	
	Rules( r.m_Rules );
	m_bInitialized = Initialize();
}
LSystem & LSystem::operator=( const LSystem & r )
{
	if( &r != this )
	{
		this->Uninitialize();
		this->m_nMaxGenerations		= r.m_nMaxGenerations;
		this->m_rAngle				= r.m_rAngle;
		this->m_rInitialSegLength	= r.m_rInitialSegLength;	
		this->m_vInitialPosition	= r.m_vInitialPosition;
		this->m_vNextDirection		= r.m_vNextDirection;
		this->_pTree				= r._pTree;

		this->Rules( r.m_Rules );
		this->m_bInitialized = Initialize();	
	}

	return *this;
}
LSystem::~LSystem()
{
	Uninitialize();
}
	
bool LSystem::Initialize()
{
	using AntiMatter::AppLog;

	// if there are no rules, make some up
	if( m_Rules.size() == 0 )
	{
		AppLog::Ref().LogMsg("LSystem has no ruleset specified, using hard coded initial conditions and creating config file");	

		m_Rules.push_back("F+F+[2]");
		m_Rules.push_back("F-");

		m_nMaxGenerations	= 14;
		m_rAngle			= 25.0f;
		m_rInitialSegLength = 10.0f;
		m_vInitialPosition	= glm::vec3(0, 77, 0);

		this->PersistSet( g_Cfg.LSystemCfg() );
	}

	if( ! ValidateRulesets() )
	{
		AppLog::Ref().LogMsg( "%s failed to Initialize. Error in Ruleset", __FUNCTION__ ); 
		Uninitialize();
		return false;
	}

	bool bSuccess = GenerateLSystem();
	
	if( ! bSuccess )
	{
		AppLog::Ref().LogMsg( "%s failed to Initialize.", 
			__FUNCTION__
			/*_com_error(hr).ErrorMessage()*/
		);
		Uninitialize();
	}

	return bSuccess;
}
void LSystem::Uninitialize()
{
	// clear down the rules dictionary
	// clear the LSystemData
	m_bInitialized		= false;

	if( m_pRootNode )
	{
		m_pRootNode->DeleteChildren();
		delete m_pRootNode;

		m_pRootNode	= NULL;
	}
		
	m_nMaxGenerations	= 0;
	m_rAngle			= 0;
	m_rInitialSegLength = 0;
	m_Rules.clear();
}

bool LSystem::ValidateRulesets()
{
	// ensure that each rule contains only valid characters and references only available rule nums
	using std::string;

	string	sValidChars = "Ff+-[]";
	char	sBuffer[2];
	int		nNumRules = m_Rules.size();

	// include rule nums in the string of valid chars for the ruleset
	for( int n = 0; n < nNumRules; n ++ )
	{
		_itoa_s( n+1, sBuffer, 10);
		sValidChars += string(sBuffer);
	}

	// iterate each rule string return false if invalid char encountered
	for( unsigned int n = 0; n < m_Rules.size(); n ++ )
	{
		string sRule	= m_Rules[n];

		// compare each character in the next rule string with the 
		// set of valid chars
		for( unsigned int j = 0; j < sRule.length(); j ++ )
		{
			if( sValidChars.find(sRule[j]) == string::npos )
				return false;
		}
	}

	return true;
}
bool LSystem::GenerateLSystem()
{
	// generate LSystem dataset, up to and including max generations
	InterpretRule( NULL, m_Rules[0] );

	return true;
}

void LSystem::InterpretRule( Segment* pCurrent, const std::string & sRule )
{
	// recursive function that generates LSystem tree segments

	using std::string;	
	using glm::vec3;
	using glm::normalize;
	using glm::rotate;	
	using glm::rotateX;
	using glm::rotateZ;

	Segment *	pSeg			= pCurrent;
	vec3		vOrientation	= pSeg ? pSeg->Orientation() : glm::vec3(0,1,0);
	

	for( unsigned int n = 0; n < sRule.length(); n ++ )
	{
		if( pSeg )
			if( pSeg->Generation() >= m_nMaxGenerations )
				break;

		char cNextChar = sRule[n];
		switch( cNextChar )
		{
		case 'F':	// generate a new segment
			{				
				if( m_pRootNode )
				{
					pSeg = AddSegment( pSeg );					
				}
				else
				{
					m_pRootNode = AddSegment( nullptr );
					pSeg		= m_pRootNode;
				}

				m_rAngle = 0.0f;

				if( ! pSeg )
					return;
			}
			break;

		case '+':
			{
				m_rAngle = 10.0f;
			}
			break;

		case '-':
			{
				m_rAngle = -10.0f;				
			}
			break;

		case '[':	// sub branch rule
			{
				string sSubRule;
				unsigned int nClosePos = 0;
				IdentifySubBranchRules( sRule, n, nClosePos, sSubRule );

				if( sSubRule.length() > 0 )
				{	
					if( nClosePos > n )
						n = nClosePos;

					InterpretRule( pSeg, sSubRule );
				}				
			}
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			int		nRuleIndex	= atoi(&cNextChar) - 1;
			string	sSubRuleset	= m_Rules[nRuleIndex];

			if( sSubRuleset.length() > 0 )							
				InterpretRule(pSeg, sSubRuleset);			

			break;
		}		
	}
}
bool LSystem::IdentifySubBranchRules( const std::string & sRule, const int nOpenPos, int nClosePos, std::string & sSubRule )
{
	// find first closing brace occurrence after the opening brace pos
	nClosePos = sRule.find( "]", nOpenPos );

	if( nClosePos == string::npos )
		return false;

	// Determine how many more opening braces occur after the first opening brace, but before the first closing brace
	int n = nOpenPos + 1;
	int nNumOpens = 0;
	
	while( n < nClosePos )
	{
		n = sRule.find( "[", n );

		if( n == string::npos || n > nClosePos )
		{
			n = nClosePos;
			break;
		}
		else
		{
			nNumOpens ++;
			n++;
		}
	}

	if( nNumOpens )
	{
		// find the nth closing brace after the first you'd encountered
		// that's the closing brace that matches the nOpenPos brace.		
		for( int p = 0; p < nNumOpens; ++ p )
		{
			nClosePos = sRule.find( "]", nClosePos );

			if( nClosePos == string::npos )
				return false;
		}
	}
	else
		nClosePos--;

	// sSubRule.resize( nClosePos - nOpenPos );
	// std::copy( sRule.at( nOpenPos+1 ), sRule.at( nClosePos ), sSubRule.begin() );
	sSubRule = sRule.substr( nOpenPos+1, nClosePos );

	return true;
}
Segment* LSystem::AddSegment( Segment* pParent )
{	
	Segment* pSeg = new (std::nothrow)
					Segment( pParent, _pTree, m_rAngle );

	if( pParent )
	{		
		if( pSeg )
			pParent->AddChild( pSeg );		
	}
	else
	{
		if( ! m_pRootNode )
			m_pRootNode = pSeg;
	}

	return pSeg;
}
void LSystem::Rules( const RulesDictionary & r )
{
	// copies the arg data into the current rules dictionary
	using std::vector;
	using std::string;

	m_Rules.clear();

	for( vector<string>::const_iterator n = r.begin(); n != r.end(); n ++ )	
		m_Rules.push_back( *n );
}
	
void LSystem::PersistSet( const std::string & sFile )
{
	using std::ofstream;
	using std::string;
	using AntiMatter::AppLog;

	std::ofstream out;
	try
	{
		out.open( sFile.c_str(), std::ios::out );

		if( out.good() )
		{
			out << *this;
			out.close();
		}
	}
	catch(...)
	{		
		AppLog::Ref().LogMsg( "%s: unanticipated exception caught", __FUNCTION__ );
		ATLASSERT(0);
	}
}

std::ofstream & operator << ( std::ofstream & out, const LSystem & r )
{
	out << "l-system\n";
	out << "max-gen "	<< r.MaxGenerations()	<< "\n";
	out << "angle "		<< r.Angle()			<< "\n";
	out << "seg-len "	<< r.InitialSegLength() << "\n";
	out << "initpos-x "	<< r.InitialPos().x		<< "\n";
	out << "initpos-y "	<< r.InitialPos().y		<< "\n";
	out << "initpos-z "	<< r.InitialPos().z		<< "\n";

	for( unsigned int n = 0; n < r.Rules().size(); n ++ )
	{
		out << r.Rules()[n].c_str() << " ";
	}
	out << std::endl;

	return out;
}
std::ifstream & operator >> ( std::ifstream & in, LSystem & r )
{
	using std::string;
	using std::vector;
	using glm::vec3;

	char			sBuffer[1024];
	vector<string>	vsRules;

	int				nMaxGenerations;
	float			rAngle;
	float			rInitialSegLen;
	float			x, y, z;

	in >> sBuffer;			// ignore this, it's just for human readability

	in >> sBuffer >> nMaxGenerations;
	in >> sBuffer >> rAngle;
	in >> sBuffer >> rInitialSegLen;
	in >> sBuffer >> x;
	in >> sBuffer >> y;
	in >> sBuffer >> z;

	while( in.good() )
	{
		in >> sBuffer;

		if( string(sBuffer).length() > 0 )
			vsRules.push_back(sBuffer);
	}
		
	r.Uninitialize();
	r.MaxGenerations( nMaxGenerations );
	r.Angle( rAngle );
	r.InitialSegLength( rInitialSegLen );
	r.InitialPosition( glm::vec3(x,y,z) );
	r.Rules( vsRules );
	r.Initialize();

	return in;
}
