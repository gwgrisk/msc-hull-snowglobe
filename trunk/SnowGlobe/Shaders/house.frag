
#version 410

in	vec2	TexCoord;
in	vec3	LightIntensity;

out	vec4	FragColor;

uniform	sampler2D	tex;

void main()
{
	vec4 TexColor	= texture( tex, TexCoord );
	FragColor		= TexColor * vec4(LightIntensity, 1.0);
}