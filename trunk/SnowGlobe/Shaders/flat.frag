
#version 400

//  This "flat" qualifier indicates that no interpolation of the value is to be done before it reaches the fragment shader.
flat in vec4 vLightIntensity;

out vec4 FragColour;

void main()
{	
	FragColour = vLightIntensity;
}