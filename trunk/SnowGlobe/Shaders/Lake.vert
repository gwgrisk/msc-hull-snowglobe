
#version 400

// -- attributes
in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;

// -- outputs to fragment shader stage
out vec2 TexCoord;

// -- uniforms
uniform mat4	mMVP;
uniform mat3	mNormal;

// -- Functions

void main()
{	
	// Texture co-ordinate (passed to frag shader)
	TexCoord = VertexTexCoord;

	// vertex position
	gl_Position = mMVP * vec4( VertexPosition, 1.0 );
}

