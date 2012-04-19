
/*
	Useful links about LSystems
	http://en.wikipedia.org/wiki/L-system
	http://nodebox.net/code/index.php/L-system
	http://www.biologie.uni-hamburg.de/b-online/e28_3/lsys.html#D0L
	http://www.opentissue.org/wikitissue/index.php/Main_Page
		
	LSystem Grammar Rules
	F : the system draws a segment here (a rectangle by default).
	+ : rotate counterclockwise by lsystem.angle.
	- : rotate clockwise by lsystem.angle.
	[ : works like the push() command, starting a subbranch.
	] : works like the pop() command, ending the subbranch.
*/

#pragma once

#include <string>
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <glm\glm.hpp>

class Segment;

typedef std::vector<std::string>		RulesDictionary;
typedef RulesDictionary::iterator		RItr;


class LSystem
{
private:
	bool				m_bInitialized;	
	int					m_nMaxGenerations;
	RulesDictionary		m_Rules;
	Segment *			m_pRootNode;
	float				m_rAngle;
	float				m_rInitialSegLength;
	glm::vec3			m_vInitialPosition;  /*TODO: initialize via ctor*/

private:
	bool	Initialize();	

	bool	ValidateRulesets();
	HRESULT	GenerateLSystem();
	void	InterpretRule( Segment* pCurrent, const std::string & sRule );
	bool	IdentifySubBranchRules( const std::string & sRule, const int nOpenPos, int nClosePos, std::string & sSubRule );

	Segment* AddSegment( Segment* pParent, const glm::mat4 & mModel );
	void Rules( const RulesDictionary & vsRules );

public:
	LSystem();
	LSystem( const std::string & sCfgFile );
	LSystem( const int & nMaxGenerations, const float rAngle, const float rInitSegLen, const glm::vec3 & vInitialPos, const std::vector<std::string> & sRuleset );
	LSystem( const LSystem & );
	LSystem & operator=( const LSystem & );
	~LSystem();

	void Uninitialize();

	const bool Initialized() const			{ return m_bInitialized; }
	Segment* LSystemData() const			{ return m_pRootNode; }		// expose LSystem dataset
	const int MaxGenerations() const		{ return m_nMaxGenerations; }	
	const RulesDictionary & Rules() const	{ return m_Rules; }
	const float Angle() const				{ return m_rAngle; }
	const float InitialSegLength() const	{ return m_rInitialSegLength; }
	const glm::vec3 & InitialPos() const	{ return m_vInitialPosition; }

	void PersistSet( const std::string & sFile );	
};

// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const LSystem & r );
std::istream & operator >> ( std::istream & in, LSystem & r );
