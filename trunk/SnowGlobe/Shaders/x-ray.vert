
#version 400

// inputs
in vec3 VertexPosition;
in vec3 VertexNormal;
in vec4 VertexColor;

// uniforms
uniform mat4	mModelView;
uniform mat4	mMVP;
uniform mat3	mNormal;

// outputs
out vec3		tNormal;
out vec4		tIncidence;
out vec4		Color;


void main()
{
 	// position & normal transformed into view space
	vec4 tPos		= mModelView * vec4( VertexPosition, 1.0 );	
	tNormal			= normalize(mNormal * VertexNormal);

	// incidence vector
	tIncidence = normalize(tPos - vec4(0.0));

	// Vertex colour
	Color = vec4(VertexColor.rgb, 0.5);

	// Vertex position translated into clip space
    gl_Position	= mMVP * vec4(VertexPosition, 1.0);
}
