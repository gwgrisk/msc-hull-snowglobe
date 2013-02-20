
#pragma once

#include <string>
#include <map>

class Material;

class WavefrontMtl
{
private:
	static bool ValidateFile( const std::string & sFile );	

public:
	static bool ParseFile( const std::string & sFile, std::map<std::string, Material> & materials );	
};