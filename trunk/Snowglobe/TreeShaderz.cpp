
#include "StdAfx.h"

#include "TreeShaderz.h"
#include <fstream>
#include <AntiMatter\ShellUtils.h>

TreeShaderz::TreeShaderz() : 
	m_bInitialized	( false )
{
	Initialize();
}
TreeShaderz::~TreeShaderz()
{
	Uninitialize();
}

bool TreeShaderz::Initialize()
{
	using AntiMatter::Shell::FileExists;
	using AntiMatter::AppLog;
	using namespace std;

	if( m_bInitialized )
		return true;

	m_Shaders.resize( (int)BumpTextured + 1 );

	if( FileExists( g_Cfg.TreeShadersCfg() ) )
	{
		ifstream in;
		in.open( g_Cfg.TreeShadersCfg().c_str(), std::ios::in );

		if( in.good() )
		{
			in >> *this;
			in.close();

			m_bInitialized = true;
		}		
	}
	else
	{
		Vert( WireFrame,		string("tree-wireframe.vert") );
		Frag( WireFrame,		string("tree-wireframe.frag") );
		Vert( Flat,				string("tree-flat.vert") );
		Frag( Flat,				string("tree-flat.frag") );
		Vert( Smooth,			string("tree-smooth.vert") );
		Frag( Smooth,			string("tree-smooth.frag") );		
		Vert( SmoothTextured,	string("tree-smooth-textured.vert") );
		Frag( SmoothTextured,	string("tree-smooth-textured.frag") );	
		Vert( BumpTextured,		string("tree-bump-textured.vert") );
		Frag( BumpTextured,		string("tree-bump-textured.frag") );

		ShaderProg( WireFrame,		NULL );
		ShaderProg( Flat,			NULL );
		ShaderProg( Smooth,			NULL );
		ShaderProg( SmoothTextured, NULL );
		ShaderProg( BumpTextured,	NULL );	
			

		ofstream out;
		out.open( g_Cfg.TreeShadersCfg().c_str(), std::ios::out );

		if( out.good() )
		{
			out << *this;
			out.close();			
		}

		m_bInitialized = true;
	}

	return m_bInitialized;
}
void TreeShaderz::Uninitialize()
{
	// iterate the vector
	// delete shaderprogram objects

	for( unsigned int n = 0; n < m_Shaders.size(); n ++ )
	{
		ShaderProgram* pNext = m_Shaders[n].ShaderProg();
		if( pNext )
		{
			delete pNext;
			pNext = NULL;

			m_Shaders[n].ShaderProg(NULL);
		}
	}


}

std::ostream & operator << ( std::ostream & out, const TreeShaderz & r )
{
	out << "TreeShaderz" << " ";
	
	for( int n = WireFrame; n < BumpTextured; n ++ )
	{
		out << r.Vert( (eTreeShaders)n ) << " ";
		out << r.Frag( (eTreeShaders)n ) << " ";
	}

	out << std::endl;

	return out;
}
std::istream & operator >> ( std::istream & in, TreeShaderz & r )
{	
	using std::string;

	string	sVert;
	string	sFrag;
	char	sBuffer[100];

	in >> sBuffer; // Ignore

	for( int n = WireFrame; n < BumpTextured; n ++ )
	{
		in >> sBuffer;
		sVert = sBuffer;

		in >> sBuffer;
		sFrag = sBuffer;

		r.Vert( (eTreeShaders)n, sVert );
		r.Frag( (eTreeShaders)n, sFrag );
		r.ShaderProg( (eTreeShaders)n, NULL );
	}

	return in;
}
