
#include "stdafx.h"
#include "Actor.h"
#include "WavefrontObj.h"
#include "slowglobe-cfg.h"
#include "VertexClass.h"

#include <vector>
#include <map>
#include <algorithm>

#include <sstream>
#include <iostream>

Actor::Actor()
{
}

Actor::Actor( const std::string & sWavefrontObjFile )
{
	m_bInitialized = InitByWavefrontObj( sWavefrontObjFile );
}

Actor::Actor( const Actor & r )
{
}

Actor::~Actor()
{
}

bool Actor::InitByWavefrontObj( const std::string & s )
{
	using std::string;

	_obj = WavefrontObj( s );

	if( ! _obj.Initialized() )
		return false;	
	
	if( ! StealWavefrontData() )
		return false;

	if( ! InitializeVbo() )
		return false;

	m_pVbo = new (std::nothrow) Vbo<CustomVertex>();

	/*
	The Wavefront obj format creates the following vectors:
	std::vector<glm::vec3>			m_Vertices;
	std::vector<glm::vec3>			m_Normals;
	std::vector<glm::vec2>			m_TexCoords;
	std::map<std::string, Material>	m_materials;

	std::vector<Face>				m_Faces;		( wraps a vector of Index)
	std::vector<SubGroup>			m_Groups;		( named collection of faces)	
	
	The faces and subgroups are useful, since they allow you to render an object
	efficiently:
	a subgroup is a collection of faces with the same render configuration
	so rendering by subgroup will allow you to render with the least amount 
	of argument changes to the shader(s).
	
	*/

	return true;
}

bool Actor::StealWavefrontData()
{
	// convert wavefront fmt data into vector<CustomVertex> format
	// with an accompanying vector<indices>
	const real r3Size	= sizeof(real)*3.0f;
	const real r2Size	= sizeof(real)*2.0f;

	typedef std::map< unsigned int, unsigned int > HashToCustVertexMap;

	HashToCustVertexMap cvMap;

	unsigned int nCvIndex = 0;

	std::vector<CustomVertex> vCustVerts;
	vCustVerts.reserve( _obj.Vertices().size() );

	auto & verts		= _obj.Vertices();
	auto & norms		= _obj.Normals();
	auto & texcoords	= _obj.TexCoords();
	auto & vFaces		= _obj.Faces();
	auto & vSubGroups	= _obj.SubGroups();
	
	// re-create the v/vn/vt string and hash it
	// search for the hash numeric value in the std::map
	std::for_each(
		vFaces.begin(), vFaces.end(),
		[&]( WavefrontObj::Face f )
		{
			WavefrontObj::Index i = f.Indices()[0];
			WavefrontObj::Index j = f.Indices()[1];
			WavefrontObj::Index k = f.Indices()[2];

			std::stringstream ss;
			ss << i.Vi() << "/" << i.Ni() << "/" << i.Ti();
			HashedString hs( ss.str().c_str() );

			auto itr = cvMap.find( hs.Hash() );

			if( itr != cvMap.end() )
			{
				// found one...
			}
		}
	);	

	return true;
}
bool Actor::InitializeVbo()
{
	using AntiMatter::AppLog;

	unsigned int nVerts;	
	unsigned int nIndices;

	// m_pVbo = new (std::nothrow) Vbo<CustomVertex> ( 
	// 	geometry.VertCount(), 
	// 	geometry.Vertices(), 
	// 	geometry.IndexCount(), 
	// 	geometry.Indices() 
	// );

	if( ! m_pVbo->Initialized() )
	{
		AppLog::Ref().LogMsg( "%s failed to initialize vertex buffer for object geometry", __FUNCTION__ );
		return false;
	}

	return true;
}