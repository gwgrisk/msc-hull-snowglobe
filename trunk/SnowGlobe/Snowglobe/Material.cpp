
#include "stdafx.h"
#include "Material.h"
#include <glm\glm.hpp>

std::ostream & operator << ( std::ostream & out, const Material & r )
{
	out << "material" << " ";
	out << r.Ka().r << " " << r.Ka().g << " " << r.Ka().b << " ";
	out << r.Kd().r << " " << r.Kd().g << " " << r.Kd().b << " ";
	out << r.Ks().r << " " << r.Ks().g << " " << r.Ks().b << " ";
	out << r.Shininess() << std::endl;

	return out;
}
std::istream & operator >> ( std::istream & in, Material & r )
{
	using glm::vec3;

	char sBuffer[1024];
	in >> sBuffer;

	vec3 vKa;
	vec3 vKd;
	vec3 vKs;
	float rShine;

	if( strcmp(sBuffer, "material") == 0 )
	{
		in >> vKa.r >> vKa.g >> vKa.b;
		in >> vKd.r >> vKd.g >> vKd.b;
		in >> vKs.r >> vKs.g >> vKs.b;
		in >> rShine;

		r.Ka(vKa);
		r.Kd(vKd);
		r.Ks(vKs);
		r.Shininess( rShine );
	}	

	return in;
}