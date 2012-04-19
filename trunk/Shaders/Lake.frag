
#version 400

// -- inputs from previous pipeline stage
in vec2		TexCoord;

// -- inputs (uniforms)
uniform sampler2D		tex;
uniform sampler2D		texAlpha;

// -- outputs
out vec4 FragColour;

void main() 
{
	vec4 alpha = texture( texAlpha, TexCoord );
	
	FragColour = vec4(1.0, 1.0, 1.0, 1.0);

	if( alpha.r < 0.15 )
		discard;	

	FragColour = texture( tex, TexCoord );	
}