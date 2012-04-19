
// simplest possible vertex shader

#version 410

layout (location = 0) in	vec3 VertexPosition;
layout (location = 1) in	vec3 VertexNormal;
layout (location = 2) in	vec2 VertexTexCoord;

uniform mat4 mMVP;

void main()
{		
	gl_Position = mMVP * vec4(VertexPosition, 1.0);
}