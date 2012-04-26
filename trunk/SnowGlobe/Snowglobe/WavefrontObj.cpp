
#include "StdAfx.h"
#include "WavefrontObj.h"
#include "WavefrontMtl.h"
#include "Material.h"

#include <glm\glm.hpp>

#include <AntiMatter\AppLog.h>
#include <AntiMatter\ShellUtils.h>

#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>

#include <iostream>
#include <fstream>
#include <sstream>


WavefrontObj::WavefrontObj( const std::string & sFile ) : 
	m_nCurrentGroupIndex	( 0 )
{
	// ensure file exists / has correct extension
	if( ! ValidateFile(sFile) )
		return;

	m_sFilename = sFile;

	// parse the file
	ParseFile();
}
WavefrontObj::~WavefrontObj()
{

}

bool WavefrontObj::ValidateFile( const std::string & sFile )
{
	if ( ! FileExists(sFile) )
		return false;

	if( ! FileHasObjExtension(sFile) )
		return false;	

	return true;
}
bool WavefrontObj::FileExists( const std::string & sFile )
{
	// convert the filename to wchar_t format for use in win32 call
	// check for file existance
	using namespace std;

	bool bExists = false;
	wstring wsFile;

	wsFile.resize( sFile.length() + 1 );
	std::copy( sFile.begin(), sFile.end(), wsFile.begin() );

	bExists = ( GetFileAttributes( wsFile.c_str() ) == INVALID_FILE_ATTRIBUTES ? false : true );
	
	return bExists;
}
bool WavefrontObj::FileHasObjExtension( const std::string & sFile )
{
	using namespace std;

	string sExn;
	bool bExists = false;

	sExn.resize(4);
	std::copy( sFile.end()-4, sFile.end(), sExn.begin() );

	if( sExn.compare(".obj") == 0 )
		bExists = true;

	return bExists;
}
	
void WavefrontObj::ParseFile()
{
	using namespace std;
	using AntiMatter::AppLog;

	ifstream	stream;
	string		sToken;	

	try
	{
		stream.open( m_sFilename.c_str(), std::ios::in );

		if( stream.is_open() )
		{
			while( stream.good() )
			{
				stream >> sToken;
				transform( sToken.begin(), sToken.end(), sToken.begin(), tolower );

				if( sToken.compare("f") == 0 )				// face
					ParseFace(stream);

				else if( sToken.compare("v") == 0 )			// vertex				
					ParseVertex(stream);

				else if( sToken.compare("vn") == 0 )		// normal				
					ParseNormal(stream);
				
				else if( sToken.compare("vt") == 0 )		// texture 				
					ParseTexCoord(stream);

				else if( sToken.compare("g") == 0 )			// new object sub-group
					NewSubGroup( stream );
				
				else if( sToken.compare("s") == 0 )			// shading instuction				
					ParseComment(stream);					// ignore for now

				else if ( sToken.compare("usemtl") == 0 )	// material instruction									
					ParseUsemtl(stream);
				
				else if ( sToken.compare("mtllib") == 0 )	// parse mtl file, add material to the m_materials vector				
					ParseMtllib( stream );								

				else										// comment, junk input or empty line				
					ParseComment(stream);
			}
		}
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s exception encountered while parsing obj file %s", __FUNCTION__, m_sFilename );
	}

	if( stream.is_open() )
		stream.close();
}
void WavefrontObj::ParseVertex( std::ifstream & stream )
{
	// can't simply read this vertex into a vector object
	// it may optionally have four components instead of three
	
	// read line
	// perform split operation
	// populate v based on the number of values returned

	using namespace std;

	glm::vec3		v;
	istringstream	sstream;
	string			sLine;	

	getline( stream, sLine );

	sstream = istringstream(sLine);

	sstream >> v.x >> v.y >> v.z;
		
	m_Vertices.push_back(v);		
}
void WavefrontObj::ParseTexCoord( std::ifstream & stream )
{
	// can't simply read this vertex into a vector object
	// it may optionally have three components instead of two
	
	// read line
	// perform split operation
	// populate v based on the number of values returned

	using namespace std;

	glm::vec2		v;
	istringstream	sstream;
	string			sLine;	

	getline( stream, sLine );

	sstream = istringstream(sLine);

	sstream >> v.x >> v.y;

	m_TexCoords.push_back( v );	
}
void WavefrontObj::ParseNormal( std::ifstream & stream )
{	
	glm::vec3 v;
	stream >> v.x >> v.y >> v.z;
	m_Normals.push_back( v );
}	
void WavefrontObj::ParseFace( std::ifstream & stream )
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
		istream_iterator <string> (sls), 
		istream_iterator <string> (),
		back_inserter <vector<string>>	(sVertexTokens)
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
void WavefrontObj::ParseComment( std::ifstream & stream )
{
	// just read to the end of line (e.g. ignore comment)
	std::string sLine;
	getline( stream, sLine );
}
void WavefrontObj::ParseMtllib( std::ifstream & stream )
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

void WavefrontObj::NewSubGroup( std::ifstream & stream )
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

void WavefrontObj::ParseUsemtl( std::ifstream & stream )
{			
	std::string	sMtlName;

	stream >> sMtlName;
	m_Groups[m_nCurrentGroupIndex].SetMaterial( sMtlName );
}

void WavefrontObj::SplitToVertTexNormal( const std::string & sData, const char cSplitBy, 
										int & nVertIndex, int & nTexIndex, int & nNormIndex  )
{
	using namespace std;
	vector<string>	sElements;

	nVertIndex = nTexIndex = nNormIndex = -1;

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


