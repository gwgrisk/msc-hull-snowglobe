
#include "StdAfx.h"
#include "WavefrontObj.h"
#include "WavefrontMtl.h"
#include "Material.h"

#include <glm\glm.hpp>

#include <AntiMatter\AppLog.h>
#include <AntiMatter\Exceptions.h>
#include <AntiMatter\ShellUtils.h>
#include <AntiMatter\AlgorithmTimer.h>

#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>

#include <iostream>
#include <fstream>
#include <sstream>

WavefrontObj::WavefrontObj() :
		m_bInitialized			( false ),
		m_nCurrentGroupIndex	( 0 )
{
}
WavefrontObj::WavefrontObj( const std::string & sFile ) : 
	m_bInitialized			( false ),
	m_nCurrentGroupIndex	( 0 )
{
	// ensure file exists / has correct extension
	if( ! ValidateFile(sFile) )
		return;

	m_sFilename = sFile;

	// parse the file
	m_bInitialized = ParseFile();
}
WavefrontObj::~WavefrontObj()
{

}

bool WavefrontObj::ValidateFile( const std::string & sFile )
{
	using AntiMatter::Shell::FileExists;
	using AntiMatter::Shell::FileHasExtension;
	using std::string;

	if ( ! FileExists(sFile) )
		return false;

	bool bExtension = FileHasExtension( sFile, string("obj") );

	if( ! bExtension )
		return false;

	return true;
}
bool WavefrontObj::ParseFile()
{
	using namespace std;
	using AntiMatter::AppLog;
	using AntiMatter::Algorithm::AlgorithmTimer;

	bool			bSuccess = true;
	ifstream		stream;
	stringstream	sstream;
	string			sToken;
	string			sLineBuffer;
	AlgorithmTimer	timer;

	glm::vec2		v2;
	glm::vec3		v3;

	unsigned int	nFaces		= 0;
	unsigned int	nVertices	= 0;
	unsigned int	nTextures	= 0;
	unsigned int	nNormals	= 0;
	unsigned int	nGroups		= 0;

	try
	{
		// Read entire file from disk into memory
		stream.open( m_sFilename.c_str(), std::ios::in );

		Enforce<AppException>(  stream.is_open() && stream.good(), 
								string("failed to open file") );

		sstream << stream.rdbuf();
		stream.close();
		
		// first pass through stringstream - count faces, verts, textures, normals,
		// then reserve memory for their various groups
		timer.Start();
			
		sLineBuffer.reserve( 100 );
		sToken.reserve(10);

		while( sstream.good() )
		{
			sstream >> sToken;
			getline( sstream, sLineBuffer );

			if( sToken.compare("f") == 0 )
				nFaces ++;

			else if( sToken.compare("v") == 0 )
				nVertices ++;

			else if( sToken.compare("vn") == 0 )
				nNormals ++;

			else if( sToken.compare("vt") == 0 )
				nTextures ++;
					
			else if( sToken.compare("g") == 0 )
				nGroups ++;
		}

		m_Vertices.reserve( nVertices );
		m_Normals.reserve( nNormals );
		m_TexCoords.reserve( nTextures );
		m_Faces.reserve( nFaces );
		m_Groups.reserve( nGroups );

		timer.Stop();
		ATLTRACE(_T("time for pass one: %5.2f seconds\r\n"), timer.DurationSecs() );
		timer.Reset();
			
			
		// second pass through stringstream - parse verts, textures, normals
		timer.Start();
		sstream.clear();
		sstream.seekg(0, ios_base::beg);

		while( sstream.good() )
		{
			sstream >> sToken;
			transform( sToken.begin(), sToken.end(), sToken.begin(), tolower );

			if( sToken.compare("f") == 0 )				// face
				ParseFace(sstream);

			else if( sToken.compare("v") == 0 )			// vertex
			{
				sstream >> v3.x >> v3.y >> v3.z;
				m_Vertices.push_back(v3);
			}

			else if( sToken.compare("vn") == 0 )		// normal
			{
				sstream >> v3.x >> v3.y >> v3.z;
				m_Normals.push_back( v3 );
			}
				
			else if( sToken.compare("vt") == 0 )		// texture 
			{
				sstream >> v2.x >> v2.y;
				m_TexCoords.push_back( v2 );	
			}

			else if( sToken.compare("g") == 0 )			// new object sub-group
				NewSubGroup( sstream );
				
			else if( sToken.compare("s") == 0 )			// shading instuction				
				getline( sstream, sLineBuffer );		// ignore for now

			else if ( sToken.compare("usemtl") == 0 )	// material instruction									
				ParseUsemtl( sstream );
				
			else if ( sToken.compare("mtllib") == 0 )	// parse mtl file, add material to the m_materials vector				
				ParseMtllib( sstream );								

			else										// parse comment, junk input or empty line
				getline( sstream, sLineBuffer );
		}

		timer.Stop();
		ATLTRACE(_T("time for pass two: %5.2f seconds\r\n"), timer.DurationSecs() );
		timer.Reset();
		
	}
	catch( AppException & e )
	{
		AppLog::Ref().LogMsg("%s: %s", __FUNCTION__, e.what() );
		bSuccess = false;
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s exception encountered while parsing obj file %s", __FUNCTION__, m_sFilename );
		bSuccess = false;
	}

	return bSuccess;
}
	
void WavefrontObj::ParseFace( std::stringstream & stream )
{
	// valid face lines include:
	// f v1..v3
	// f v1/vt1 v2/vt2 v3/vt3 ...
	// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...
	//
	// if we've got more than 3 vertex entries, we've got a poly and things get miserable!
	//
	using namespace std;

	string				sLine;
	vector<string>		sVertexTokens;
	istringstream		sls;

	// get the line from the stream into an istringstream
	getline( stream, sLine );
	sls = istringstream(sLine);

	// split the string by spaces, store in a vector of string
	copy(
		istream_iterator <string>			(sls),
		istream_iterator <string>			(),
		back_inserter < vector<string> >	(sVertexTokens)
	);

	// determine if we've got a complex face that needs to be sub-divided into triangles
	int nNumVerts = sVertexTokens.size();

	vector<int>		vVI;
	vector<int>		vTI;
	vector<int>		vNI;

	vVI.resize( nNumVerts );
	vTI.resize( nNumVerts );
	vNI.resize( nNumVerts );
	
	if( nNumVerts == 3 )
	{		
		Face	triFace;

		for( int i = 0; i < nNumVerts; i ++ )
		{
			SplitToVertTexNormal( sVertexTokens[i], '/', vVI[i], vTI[i], vNI[i] );
			triFace.PushIndex( Index( vVI[i], vTI[i], vNI[i] ) );
		}		
				
		m_Faces.push_back( triFace );
	}
	else if( nNumVerts == 4 )
	{
		Face triFace1;		// 2 triangles from one quad
		Face triFace2;

		for( int i = 0; i < nNumVerts; i ++ )		
			SplitToVertTexNormal( sVertexTokens[i], '/', vVI[i], vTI[i], vNI[i] );			
			
		for( int j = 0; j < 3; j ++ )
		{
			triFace1.PushIndex( Index( vVI[j], vTI[j], vNI[j] ) );
			triFace2.PushIndex( Index( vVI[j+1], vTI[j+1], vNI[j+1] ) );
		}

		m_Faces.push_back( triFace1 );
		m_Faces.push_back( triFace2 );
	}
	else if( nNumVerts > 4 )
	{
		// we've got a complex poly.  Unsupported :)
		ATLTRACE(_T("We've encountered a complex polygon in this file, this parser doesn't support them."));
		ATLASSERT(0);
	}
	else if( nNumVerts < 3 )
	{
		// TODO: ignore line, flag as invalid file?
		ATLTRACE(_T("We've encountered a polygon with < 3 vertices.  How can this be!?  invalid file?"));
		ATLASSERT(0);
	}	
}
void WavefrontObj::ParseMtllib( std::stringstream & stream )
{
	using std::string;

	string sMtlFilename;
	string sFilePath;
	string sCompleteMtlFilePath;

	stream >> sMtlFilename;

	// Prepend the current folder from the obj file filename
	AntiMatter::Shell::GetFilePath( m_sFilename, sFilePath );
	
	sCompleteMtlFilePath = sFilePath + sMtlFilename;

	WavefrontMtl::ParseFile( sCompleteMtlFilePath, m_materials );
}
void WavefrontObj::ParseUsemtl( std::stringstream & stream )
{			
	std::string	sMtlName;

	stream >> sMtlName;
	m_Groups[m_nCurrentGroupIndex].SetMaterial( sMtlName );
}
void WavefrontObj::NewSubGroup( std::stringstream & stream )
{
	using std::string;
	using std::istringstream;
	using AntiMatter::AppLog;

	SubGroup		s;
	string			sGrpName;
	
	if( stream.good() )
	{
		// subgroup name is the next token in the stream
		stream >> sGrpName;

		s.SubGroupName( sGrpName );
		s.SetStartFaceIndex( m_Faces.size() );
		m_Groups.push_back( s );

		m_nCurrentGroupIndex = m_Groups.size() == 0 ? 0 : m_Groups.size() - 1;
	}
}

void WavefrontObj::SplitToVertTexNormal( const std::string & sData, const char cSplitBy, 
										int & nVertIndex, int & nTexIndex, int & nNormIndex  )
{
	using namespace std;
	vector<string>	sElements;

	nVertIndex = nTexIndex = nNormIndex = -1;

	sElements.reserve(3);

	SplitString( sData, cSplitBy, sElements );	

	if( sElements.size() == 3 )
	{
		nVertIndex	= atoi(sElements[0].c_str());
		nTexIndex	= atoi(sElements[1].c_str());
		nNormIndex	= atoi(sElements[2].c_str());
	}	
	else if ( sElements.size() == 2 )
	{
		nVertIndex	= atoi(sElements[0].c_str());
		nTexIndex	= atoi(sElements[1].c_str());
	}
	else if ( sElements.size() == 1 )
	{
		nVertIndex	= atoi(sElements[0].c_str());
	}
}

void WavefrontObj::SplitString( const std::string & sInput, const char cDelimiter, std::vector<std::string> & vElements )
{
	using namespace std;

	stringstream ss(sInput);
	string sNext;

	while( getline( ss, sNext, cDelimiter ) )
		vElements.push_back(sNext);
}
