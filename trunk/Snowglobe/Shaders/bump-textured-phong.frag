
#version 400

subroutine vec4 FragmentLightIntensity();
subroutine uniform FragmentLightIntensity LightIntensity;

// inputs from previous pipeline stage
in vec3 vVertexNormal;			// Vertex normal				(in view space)
in vec3 vViewDirection;			// direction to camera			(in view space)
in vec3 vSunDirection;			// direction to light source	(in view space)
in mat4 mSpotDirections;		// direction to each spotlight  (in view space)
in vec2 TexCoord;				// texture coordinate			( untransformed ;) )
in vec3 vVertexPos;

// output
out vec4 FragColour;

struct LightInfo
{
	vec4	Position;
	vec3	La;
	vec3	Ld;
	vec3	Ls;
	vec3	vDirection;
	float	rExponent;
	float	rCutOff;
};

struct MaterialInfo
{
	vec3  Ka;
	vec3  Kd;
	vec3  Ks;
	float rShininess;
};


uniform vec3			vViewPosition;	// camera position (view space)
uniform LightInfo		lights[5];		// light[0-3] = spotlights, light[4] = sun
uniform MaterialInfo	material;

uniform	sampler2D		tex;
uniform sampler2D		texBump;

uniform mat4			mModelView;
uniform mat3			mNormal;
uniform mat4			mMVP;

vec3 SpotlightPhong( int n, vec3 vLightDir )
{	
	vec3	s			= vLightDir;
	vec3	v			= vViewDirection;
	vec3	r			= normalize(reflect( -s, vVertexNormal ));
	float	angle		= acos( dot( -s, lights[n].vDirection ) );
	float	cutoff		= radians( clamp( lights[n].rCutOff, 0.0, 90.0 ) );		
	
	float	SdotN		= max( dot(s, vVertexNormal), 0.0 );
	vec3	ambient		= lights[n].La * material.Ka;
	vec3	diffuse		= lights[n].Ld * material.Kd * SdotN;
	vec3	specular	= vec3(0.0);

	if( SdotN > 0.0 )
	{
		specular = lights[n].Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.rShininess );
	}

	return ambient + diffuse + specular;
}

vec3 Phong( int n, vec3 vLightDir )
{
	vec3	s = vLightDir;
	vec3	v = vViewDirection;
	vec3	r = normalize(reflect( -s, vVertexNormal ));
	
	float	SdotN		= max( dot(s, vVertexNormal), 0.0 );
	vec3	ambient		= lights[n].La * material.Ka;
	vec3	diffuse		= lights[n].Ld * material.Kd * SdotN;
	vec3	specular	= vec3(0.0);

	if( SdotN > 0.0 )
	{
		specular = lights[n].Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.rShininess );
	}

	return ambient + diffuse + specular;
}

subroutine (FragmentLightIntensity)
vec4 Spotlights()
{
	vec3 vLightIntensity;

	vLightIntensity = SpotlightPhong( 0, vec3(mSpotDirections[0]) );
	vLightIntensity += SpotlightPhong( 1, vec3(mSpotDirections[1]) );
	vLightIntensity += SpotlightPhong( 2, vec3(mSpotDirections[2]) );
	vLightIntensity += SpotlightPhong( 3, vec3(mSpotDirections[3]) );		

	return vec4(vLightIntensity, 1.0);
}

subroutine( FragmentLightIntensity )
vec4 Sunlight()
{		
	return vec4( Phong( 4, vSunDirection ), 1.0 );
}

void main()
{
	FragColour	= texture2D( tex, TexCoord ) * LightIntensity();
}