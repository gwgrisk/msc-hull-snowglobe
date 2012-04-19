
// couldn't get the textures to display properly,
// so just drawing coloured squares.

// As we are rendering GL_POINT primitives, the texture coordinate
// is automatically available through gl_PointCoord;

#version 410

uniform sampler2D	snowflake;
in float			Transparency;

out vec4 vFragColour;

void main()
{
	vFragColour		= texture2D( snowflake, gl_PointCoord );
	vFragColour.a	= Transparency;	
}
