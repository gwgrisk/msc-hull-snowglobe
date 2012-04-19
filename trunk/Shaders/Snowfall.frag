
#version 410

uniform sampler2D	Tex;
in float			Transparency;

out vec4 FragColour;

void main()
{
	FragColour = texture( Tex, gl_PointCoord );
	FragColour.a *= Transparency;	
}
