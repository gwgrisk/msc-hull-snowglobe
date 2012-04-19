
#version 330
precision highp float;

in vec4 VertexPosition;
in vec3 VertexNormal;

out Vertex
{
	vec3 Normal;
} Vertex;

void main()
{
	gl_Position		= VertexPosition;
	Vertex.Normal	= VertexNormal;
}