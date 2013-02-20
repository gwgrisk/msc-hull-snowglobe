//
// Wavefront OBJ file loader
// 
// References
// http://en.wikipedia.org/wiki/Wavefront_.obj_file
// 
// OBJ file format is a simple data format that represents 3D geometry alone
// e.g.
// Vertices, with (x,y,z[,w]) coordinates, w is optional.
// Texture coordinates, in (u,v[,w]) coordinates, w is optional.
// Normals in (x,y,z) form; normals might not be unit.
// Face Definitions
// Vertices are stored in counter clockwise order by default, making explicit declaration of normals unnecessary

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <glm\glm.hpp>
#include "HashedString.h"

class Material;


class WavefrontObj
{
public:
	class Index
	{
	private:
		int		m_V;	// vertex index
		int		m_T;	// texture index
		int		m_N;	// normal index

	public:
		Index( const int v = 0, const int t = -1, const int n = -1 ) : 
			m_V(v),
			m_T(t),
			m_N(n)
		{
		}

		const int Vi() const { return m_V; }
		const int Ti() const { return m_T; }
		const int Ni() const { return m_N; }
	};
	class Face
	{
	private:
		std::vector<Index>		m_Indices;

	public:
		const std::vector<Index> & Indices()
		{
			return m_Indices;
		}
		void PushIndex( const Index & v )
		{
			m_Indices.push_back( v );
		}
		void PopIndex()
		{
			m_Indices.pop_back();
		}
		void Clear()
		{
			m_Indices.clear();
		}

		int NumIndices()
		{
			return m_Indices.size();
		}
	};
	class SubGroup
	{
	private:
		std::string		m_sName;
		std::string		m_sMaterialName;
		unsigned int	m_nStartFaceIndex;
		unsigned int	m_nNumFaces;

	public:
		SubGroup() :
			m_sName				(""),
			m_sMaterialName		(""),
			m_nStartFaceIndex	( 0 ),
			m_nNumFaces			( 0 )
		{
		}
		SubGroup( const std::string & sName ) :
			m_sName				( sName ),
			m_sMaterialName		( "" ),
			m_nStartFaceIndex	( 0 ),
			m_nNumFaces			( 0 )
		{
		}
		SubGroup & operator= ( const SubGroup & r )
		{
			if( this != &r )
			{
				m_sName				= r.m_sName;
				m_sMaterialName		= r.m_sMaterialName;
				m_nStartFaceIndex	= r.m_nStartFaceIndex;
				m_nNumFaces			= r.m_nNumFaces;
			}

			return *this;
		}

		void SubGroupName( const std::string & s )			{ m_sName = s; }
		const std::string & SubGroupName()					{ return m_sName; }

		void SetMaterial( const std::string & sMtlName )	{ m_sMaterialName = sMtlName;  }		
		const std::string & Material() const				{ return m_sMaterialName; }

		void SetStartFaceIndex( const unsigned int n )		{ m_nStartFaceIndex = n; }
		const unsigned int StartFaceIndex() const			{ return m_nStartFaceIndex; }

		void NumFaces( const unsigned int & r)				{ m_nNumFaces = r; }
		const unsigned int NumFaces()						{ return m_nNumFaces; }
	};
		
private:
	bool							m_bInitialized;
	std::string						m_sFilename;
	std::vector<glm::vec3>			m_Vertices;
	std::vector<glm::vec3>			m_Normals;
	std::vector<glm::vec2>			m_TexCoords;
	std::map<std::string, Material>	m_materials;

	std::vector<Face>				m_Faces;
	std::vector<SubGroup>			m_Groups;

	int								m_nCurrentGroupIndex;		// current group index

private:
	bool ValidateFile( const std::string & sFile );
	bool FileExists( const std::string & sFile );
	bool FileHasObjExtension( const std::string & sFile );

	bool ParseFile();
	void ParseFace( std::stringstream & stream );
	void ParseMtllib( std::stringstream & stream );
	void ParseUsemtl( std::stringstream & stream );

	void NewSubGroup( std::stringstream & stream );

	void SplitString( const std::string & sInput, const char cDelimiter, std::vector<std::string> & sElements );

	void SplitToVertTexNormal(	const std::string & sData, const char cSplitBy, 
								int & nVertIndex, int & nTexIndex, int & nNormIndex  );

public:
	WavefrontObj();
	WavefrontObj( const std::string & sFile );
	~WavefrontObj();

	bool Initialized() const { return m_bInitialized; }
	
	const std::vector<glm::vec3> & Vertices() const		{ return m_Vertices; }
	const std::vector<glm::vec3> & Normals() const		{ return m_Normals; }
	const std::vector<glm::vec2> & TexCoords() const	{ return m_TexCoords; }

	void MoveVerts( std::vector<glm::vec3> & l )		{ l = std::move( m_Vertices ); }
	void MoveNorms( std::vector<glm::vec3> & l )		{ l = std::move( m_Normals ); }
	void MoveTexCs( std::vector<glm::vec2> & l )		{ l = std::move( m_TexCoords ); }

	void MoveMaterials( std::map<std::string, Material> & l ) { l = std::move(m_materials); }

	const std::vector<Face> & Faces() const				{ return m_Faces; }
	const std::vector<SubGroup> & SubGroups() const		{ return m_Groups; }
};
