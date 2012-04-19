
#version 400

in vec3 tNormal;
in vec4 tIncidence;
in vec4 Color;

out vec4 FragColor;

uniform float rFallOff;

void main()
{
	float rAngle		= abs( dot( vec4(tNormal, 1.0), -tIncidence ) );	
	float rOpaqueness	= 1.0 - pow( abs(rAngle), rFallOff );

	FragColor.rgb	= Color.rgb * rOpaqueness;
	FragColor.a		= rOpaqueness;
}
