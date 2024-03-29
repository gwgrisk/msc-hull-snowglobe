
#version 400

//  This "flat" qualifier indicates that no interpolation of the value is to be done before it reaches the fragment shader.
flat in vec4 vLightIntensity;
in vec4 VertColour;

out vec4 FragColour;

void main()
{	
	FragColour = VertColour * vLightIntensity;
}