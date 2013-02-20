
#include "stdafx.h"
#include "Material.h"
#include <glm\glm.hpp>

std::ostream & operator << ( std::ostream & out, const Material & r )
{
	out << "newmtl" << " " << r.Name() << "\n";
	out << "Ns"		<< r.Ns() << "\n";
	out << "d"		<< r.D() << "\n";
	out << "Tr"		<< r.Tr() << "\n";
	out << "Tf"		<< r.Tf().r << " " << r.Tf().g << " " << r.Tf().b << "\n";
	out << "illum"	<< r.Illum() << "\n";
	out << "Ka"		<< r.Ka().r << " " << r.Ka().g << " " << r.Ka().b << "\n";
	out << "Kd"		<< r.Kd().r << " " << r.Kd().g << " " << r.Kd().b << "\n";
	out << "Ks"		<< r.Ks().r << " " << r.Ks().g << " " << r.Ks().b << "\n";
	out << "Shine"	<< r.Shininess() << std::endl;

	return out;
}
std::istream & operator >> ( std::istream & in, Material & r )
{
	using glm::vec3;
	using std::string;
	using std::transform;

	char			sBuffer[1024];
	string			sFieldName;
	string			sMtlName;
		
	unsigned int	ns		= 32;
	unsigned int	d		= 1;
	unsigned int	tr		= 0;
	glm::vec3		tf		= glm::vec3(1,1,1);
	unsigned int	illum	= 2;
	vec3			vKa		= glm::vec3(1,1,1);
	vec3			vKd		= glm::vec3(1,1,1);
	vec3			vKs		= glm::vec3(1,1,1);
	float			rShine	= 1.0f;

	in >> sBuffer;

	if( strcmp(sBuffer, "newmtl") == 0 )
	{
		while( in >> sFieldName, in.good() && sFieldName.length() > 0 )
		{
			transform( sFieldName.begin(), sFieldName.end(), sFieldName.begin(), tolower );
		
			if( sFieldName.compare("newmtl") == 0 )
				in >> sMtlName;
			
			else if( sFieldName.compare("ns") == 0 )
				in >> ns;
			
			else if( sFieldName.compare("d") == 0 )
				in >> d;
			
			else if( sFieldName.compare("tr") == 0 )
				in >> tr;
			
			else if( sFieldName.compare("tf") == 0 )
				in >> tf.r >> tf.g >> tf.b;
			
			else if( sFieldName.compare("illum") == 0 )
				in >> illum;

			else if( sFieldName.compare("ka") == 0 )
				in >> vKa.r >> vKa.g >> vKa.b;
			
			else if( sFieldName.compare("kd") == 0 )
				in >> vKd.r >> vKd.g >> vKd.b;

			else if( sFieldName.compare("ks") == 0 )
				in >> vKs.r >> vKs.g >> vKs.b;

			else if( sFieldName.compare("shine") == 0 )
				in >> rShine;
		}
		
		r.Name( sMtlName );
		r.Ka(vKa);
		r.Kd(vKd);
		r.Ks(vKs);
		r.Shininess( rShine );

		r.Ns( ns );
		r.D( d );
		r.Tr( tr );
		r.Tf( tf );
		r.Illum( illum );
	}

	return in;
}