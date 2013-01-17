// flat vertex shader

#version 400

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec4 VertexColour;

uniform mat4 mMVP;

flat out vec4 VertColour;

void main()
{
	VertColour  = VertexColour;
	gl_Position = mMVP * vec4(VertexPosition, 1.0);
}