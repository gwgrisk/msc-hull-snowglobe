
#pragma once

#include <AntiMatter\Singleton.h>
#include <iosfwd>

class SeasonalTimeline : public Singleton <SeasonalTimeline>
{
private: 
	friend class Singleton <SeasonalTimeline>;

public:
	enum Seasons { Spring, Summer, Autumn, Winter };

private:
	bool	m_bInitialized;
	float	m_rSpeed;				// 0.0f .. 5.0f
	float	m_rPrevSpeed;

	float	m_rSeasonTimeline;		// range = m_rSeasonDurataion * 4
	float	m_rSeasonDuration;		// m_rSeasonDuration should be a function of the number of frames per season?
	
public:
	SeasonalTimeline();
	SeasonalTimeline( const SeasonalTimeline & r );	
	SeasonalTimeline( const std::string & sConfigFile );
	SeasonalTimeline & operator=( const SeasonalTimeline & r );
	~SeasonalTimeline();

	void IncSpeed()							{ m_rSpeed		= (m_rSpeed < 5.0f) ? ++m_rSpeed : m_rSpeed; }
	void DecSpeed()							{ m_rSpeed		= (m_rSpeed > 0.0f) ? --m_rSpeed : m_rSpeed; }
	void Pause()							{ m_rPrevSpeed	= m_rSpeed; m_rSpeed = 0.0f; }
	void Resume()							{ m_rSpeed		= m_rPrevSpeed; }
	
	void Update( const float rSecsDelta )
	{
		if( m_rSpeed > 0.0f )
			m_rSeasonTimeline += ( rSecsDelta * m_rSpeed );
	}

	// gets
	const float Speed() const				{ return m_rSpeed; } 
	const float PrevSpeed() const			{ return m_rPrevSpeed; }
	const float SeasonTimeline() const		{ return m_rSeasonTimeline; }
	const float SeasonDuration() const		{ return m_rSeasonDuration; }
	// sets
	void Speed( const float r )				{ m_rSpeed			= r; }
	void PrevSpeed( const float r )			{ m_rPrevSpeed		= r; }
	void SeasonTimeline( const float r )	{ m_rSeasonTimeline = r; }
	void SeasonDuration( const float r )	{ m_rSeasonDuration = r; }
};
	
	
// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const SeasonalTimeline & r );
std::istream & operator >> ( std::istream & in, SeasonalTimeline & r );