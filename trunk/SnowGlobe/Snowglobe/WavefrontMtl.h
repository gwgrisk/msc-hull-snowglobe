
#pragma once

#include <vector>
#include <string>

class Material;

class WavefrontMtl
{
private:
	static bool ValidateFile( const std::string & sFile );	

public:
	static bool ParseFile( const std::string & sFile, std::vector<Material> & materials );	
};