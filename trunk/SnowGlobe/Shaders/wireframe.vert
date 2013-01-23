
// simplest possible vertex shader

#version 400

layout (location = 0) in	vec3 VertexPosition;
layout (location = 1) in	vec3 VertexNormal;
layout (location = 2) in	vec2 VertexTexCoord;

uniform mat4 mMVP;
uniform vec4 wfColour;

out vec4 colour;

void main()
{		
	colour		= wfColour;
	gl_Position = mMVP * vec4(VertexPosition, 1.0);
}