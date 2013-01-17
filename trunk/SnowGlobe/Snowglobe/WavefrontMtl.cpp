
#include "stdafx.h"
#include "WavefrontMtl.h"
#include "Material.h"

#include <glm\glm.hpp>
#include <AntiMatter\ShellUtils.h>

#include <fstream>
#include <sstream>

#include <map>
#include <utility>


bool WavefrontMtl::ValidateFile( const std::string & sFile )
{
	using AntiMatter::Shell::FileExists;
	using AntiMatter::Shell::FileHasExtension;
	using std::string;

	if( ! FileExists( sFile ) )
		return false;

	return FileHasExtension( sFile, string("mtl"));
}
bool WavefrontMtl::ParseFile( const std::string & sFile, std::map<std::string, Material> & materials )
{
	using std::ifstream;
	using std::istringstream;
	using std::string;
	using std::pair;
	using AntiMatter::AppLog;

	bool			bSuccess = false;
	ifstream		stream;
	string			sToken;

	glm::vec3		Ka			= glm::vec3(0.0);
	glm::vec3		Kd			= glm::vec3(0.0);
	glm::vec3		Ks			= glm::vec3(0.0);
	float			rShininess	= 0.0f;
	string			sName;
	unsigned int	Ns			= 0;
	unsigned int	d			= 0;
	unsigned int	Tr			= 0;
	glm::vec3		Tf			= glm::vec3(0.0);
	unsigned int	Illum		= 0;

	glm::vec3		v			= glm::vec3(0.0);
	istringstream	sstream;
	string			sLine;	


	if( ! ValidateFile( sFile ) )
	{
		AppLog::Ref().LogMsg("%s failed to validate the supplied filename while parsing material for file %s", __FUNCTION__, sFile );		
		return false;
	}

	try
	{
		stream.open( sFile.c_str(), std::ios::in );

		while ( stream.is_open() && stream.good() )
		{
			getline( stream, sLine );

			if( sLine.length() > 0 )
			{
				sstream = istringstream(sLine);
				
				sstream >> sToken;
				transform( sToken.begin(), sToken.end(), sToken.begin(), tolower );

				if( sToken.compare("#") == 0 )
					continue;
				else if ( sToken.compare("newmtl") == 0 )
					sstream >> sName;
				else if( sToken.compare("ns") == 0 )
					sstream >> Ns;
				else if( sToken.compare("d") == 0 ) 
					sstream >> d;
				else if( sToken.compare("tr") == 0 ) 
					sstream >> Tr;
				else if( sToken.compare("tf") == 0 ) 
					sstream >> Tf.x >> Tf.y >> Tf.z;
				else if( sToken.compare("illum") == 0 ) 
					sstream >> Illum;
				else if( sToken.compare("ka") == 0 ) 
					sstream >> Ka.x >> Ka.y >> Ka.z;
				else if( sToken.compare("kd") == 0 ) 
					sstream >> Kd.x >> Kd.y >> Kd.z;
				else if( sToken.compare("ks") == 0 ) 
					sstream >> Ks.x >> Ks.y >> Ks.z;
			}
			else if( sName.length() > 0 )
			{
				bSuccess = true;

				materials.insert( 
					pair <string, Material> ( sName, Material(sName, Ka, Kd, Ks, rShininess, Ns, d, Tr, Tf, Illum) )
				);

				sName		= "";
				Ka			= glm::vec3(0.0f);
				Kd			= glm::vec3(0.0f);
				Ks			= glm::vec3(0.0f);
				rShininess	= 0.0f;
				Ns			= 0;
				d			= 0;
				Tr			= 0;
				Tf			= glm::vec3(0.0f);
				Illum		= 0;
			}
		}
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s exception encountered while parsing material file %s", __FUNCTION__, sFile );
		bSuccess = false;
	}

	if( bSuccess )	
		AppLog::Ref().LogMsg("%s succeeded parsing mtl file %s", __FUNCTION__, sFile.c_str() );

	if( stream.is_open() )
		stream.close();

	return bSuccess;
}
