
#version 410

// input attributes
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

// outputs 
out vec2 TexCoord;
out vec3 LightIntensity;

// structs
struct LightInfo
{
	vec4 Position;		// make sure this is in view space! (mult by modelview before passing it in!)
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};

// uniforms
uniform LightInfo		lights[5];		// [0..3] = point lights, [4] = sun
uniform	sampler2D		tex;
uniform	sampler2D		texAlpha;

uniform vec3			Ka;
uniform	vec3			Kd;
uniform vec3			Ks;
uniform float			rShininess;


uniform mat4			mModelView;
uniform mat3			mNormal;
uniform mat4			mMVP;


vec3 Phong( int x, vec4 tPos, vec3 tNorm )
{
	// TODO: halfway vector optimization

	vec3	s = normalize( vec3(lights[x].Position - tPos) );
	vec3	v = normalize( vec3(-tPos) );
	vec3	r = reflect( -s, tNorm );

	vec3	ambient		= lights[x].La * Ka;
	float	sDotN		= max( dot(s, tNorm), 0.0 );
	vec3	diffuse		= lights[x].Ld * Kd * sDotN;
	vec3	specular	= vec3(0.0);

	if( sDotN > 0.0 )
	{
		specular = lights[x].Ls * Ks * pow( max( dot(r,v), 0.0 ), rShininess );
	}

	return ambient + diffuse + specular;
}


void main()
{
	// transform vertex position and vertex normal into view (/camera/eye) space for Phong calculation
	vec4 tPos		= mModelView * vec4( VertexPosition, 1.0 );
	vec3 tNormal	= normalize( mNormal * VertexNormal );
	LightIntensity	= vec3(0.0, 0.0, 0.0);

	// Phong is computed and accumulated once per light per _vertex_
	for( int x = 0 ; x < 5; x ++ )
		LightIntensity += Phong( x, tPos, tNormal );

	// texture UV coords are required in the fragment shader
	TexCoord = VertexTexCoord;
	
	// Position must be set for all vertex shaders
	gl_Position = mMVP * vec4(VertexPosition, 1.0);
}