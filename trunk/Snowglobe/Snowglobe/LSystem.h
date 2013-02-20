
/*
	Useful links about LSystems
	http://en.wikipedia.org/wiki/L-system
	http://nodebox.net/code/index.php/L-system
	http://www.biologie.uni-hamburg.de/b-online/e28_3/lsys.html#D0L
	http://www.opentissue.org/wikitissue/index.php/Main_Page
	
	"An L-system (or Lindenmayer system) is a set of (recursive) rules and symbols used 
	to model growth processes."

	LSystem Grammar Rules
	F : the system draws a segment here.
	+ : rotate counterclockwise by lsystem.angle.
	- : rotate clockwise by lsystem.angle.
	[ : works like the push() command, starting a subbranch.
	] : works like the pop() command, ending the subbranch.
*/

#pragma once

#include <iosfwd>
#include <fstream>
#include <vector>
#include <string>
#include <glm\glm.hpp>
#include <AntiMatter\precision.h>

class Segment;
class Tree;


class LSystem
{
	typedef std::vector<std::string> RulesDictionary;
	

private:
	bool				m_bInitialized;	
	int					m_nMaxGenerations;
	RulesDictionary		m_Rules;
	Segment *			m_pRootNode;
	float				m_rAngle;
	float				m_rInitialSegLength;
	glm::vec3			m_vInitialPosition;
	glm::vec3			m_vNextDirection;
	Tree*				_pTree;

public:	
	bool	ValidateRulesets();
	bool	GenerateLSystem();
	void	InterpretRule( Segment* pCurrent, const std::string & sRule );
	bool	IdentifySubBranchRules( const std::string & sRule, const int nOpenPos, int nClosePos, std::string & sSubRule );

	Segment* AddSegment( Segment* pParent );
	void Rules( const RulesDictionary & vsRules );

public:
	LSystem();
	LSystem( Tree* pTree, const std::string & sCfgFile );
	LSystem( Tree* pTree, const int & nMaxGenerations, const float rAngle, const float rInitSegLen, const glm::vec3 & vInitialPos, const std::vector<std::string> & sRuleset );
	LSystem( const LSystem & );
	LSystem & operator=( const LSystem & r );
	~LSystem();

	bool Initialize();
	void Uninitialize();

	const bool Initialized() const				{ return m_bInitialized; }
	Segment* LSystemData() const				{ return m_pRootNode; }		// expose LSystem dataset
	const int MaxGenerations() const			{ return m_nMaxGenerations; }	
	const RulesDictionary & Rules() const		{ return m_Rules; }
	const float Angle() const					{ return m_rAngle; }
	const float InitialSegLength() const		{ return m_rInitialSegLength; }
	const glm::vec3 & InitialPos() const		{ return m_vInitialPosition; }

	void MaxGenerations( const int n )			{ m_nMaxGenerations = n; }
	void Angle( const real r )					{ m_rAngle = r; }
	void InitialSegLength( const real r )		{ m_rInitialSegLength = r; }
	void InitialPosition( const glm::vec3 & p )	{ m_vInitialPosition = p; }

	void PersistSet( const std::string & sFile );	

};

std::ofstream & operator << ( std::ofstream & out, const LSystem & r );
std::ifstream & operator >> ( std::ifstream & in, LSystem & r );
