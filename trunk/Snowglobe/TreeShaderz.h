
/*
	TreeShaderz
	This object persists a list of vertex and fragment shader file paths for easy retrieval 
	Later, when the information is used to load the shaders, TreeShaderz can be used for easy
	storage and retrieval of each corresponding ShaderProgram pointer address
*/

#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "slowglobe-cfg.h"
#include "ShaderProgram.h"
	
enum eTreeShaders { WireFrame, Flat, Smooth, SmoothTextured, BumpTextured };


class TreeShaderz
{
private:
	class TreeShader
	{
	private:
		int				m_nShaderId;
		std::string		m_sVertShader;
		std::string		m_sFragShader;
		ShaderProgram*	m_pShader;
		
	public:
		TreeShader() :
			m_nShaderId	( 0 ),
			m_pShader	( NULL )
		{
		}
		TreeShader( const int nId, const std::string & sVert, const std::string & sFrag, ShaderProgram * pShader = NULL ) : 
			m_nShaderId		( nId ),
			m_sVertShader	( sVert ),
			m_sFragShader	( sFrag ),
			m_pShader		( pShader )
		{
		}
		~TreeShader()
		{
		}

		// get
		const int ShaderId() const				{ return m_nShaderId; }
		const std::string & Vert() const		{ return m_sVertShader; }
		const std::string & Frag() const		{ return m_sFragShader; }

		// set
		ShaderProgram* ShaderProg()				{ return m_pShader; }
		void ShaderProg( ShaderProgram * p )	{ m_pShader = p; }
		void Vert( const std::string & s )		{ m_sVertShader = s; }
		void Frag( const std::string & s )		{ m_sFragShader = s; }
	};

private:
	std::vector<TreeShader>		m_Shaders;
	bool						m_bInitialized;

private:
	bool Initialize();
	void Uninitialize();

public:
	TreeShaderz();
	~TreeShaderz();

	const bool Initialized() const 
	{
		return m_bInitialized;
	}

	// set	
	void ShaderProg( eTreeShaders e, ShaderProgram* p )
	{
		m_Shaders[e].ShaderProg(p);
	}
	void Vert( eTreeShaders e, const std::string & sV )
	{
		m_Shaders[e].Vert( sV );
	}
	void Frag( eTreeShaders e, const std::string & sF )
	{
		m_Shaders[e].Frag( sF );
	}

	// get
	ShaderProgram* ShaderProg( eTreeShaders e )
	{
		return m_Shaders[e].ShaderProg();
	}
	const int ShaderId( eTreeShaders e )const { return m_Shaders[e].ShaderId(); }
	const std::string & Vert( eTreeShaders e ) const
	{
		return m_Shaders[e].Vert();
	}
	const std::string & Frag( eTreeShaders e ) const
	{
		return m_Shaders[e].Frag();
	}
};

// auxilary persistence functions
std::ostream & operator << ( std::ostream & out, const TreeShaderz & r );
std::istream & operator >> ( std::istream & in, TreeShaderz & r );
