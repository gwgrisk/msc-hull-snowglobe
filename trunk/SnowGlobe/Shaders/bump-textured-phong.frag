
#version 400

subroutine vec4 FragmentLightIntensity();
subroutine uniform FragmentLightIntensity LightIntensity;

// inputs from previous pipeline stage
in vec3 vVertexNormal;			// Vertex normal				(in view space)
in vec3 vViewDirection;			// direction to camera			(in view space)
in vec3 vSunDirection;			// direction to light source	(in view space)
in mat4 mSpotDirections;		// direction to each spotlight  (in view space)
in vec2 TexCoord;				// texture coordinate
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

uniform mat4			mModelView;
uniform mat3			mNormal;
uniform mat4			mMVP;

uniform	sampler2D		tex;
uniform sampler2D		texBump;


vec4 SpotlightPhong( int n, vec3 vLightDir )
{	
	vec3 vTexNormal		= normalize( ( texture2D(texBump, TexCoord).xyz * 2.0 ) - 1.0 );
	float NdotL			= max( 0.0, dot( vTexNormal, vLightDir ));

	vec3 r				= normalize( ( ( 2.0 * vTexNormal ) * NdotL ) - vLightDir );
	vec3 v				= normalize( vViewDirection );
	float RdotV			= max( 0.0, dot( r, v ) );

	float angle			= acos( dot( -vLightDir, lights[n].vDirection ) );
	float cutoff		= radians( clamp( lights[n].rCutOff, 0.0, 90.0 ) );		
	
	vec3 ambient	= lights[n].La * material.Ka;

	if( angle > cutoff )
	{		
		vec3 diffuse	= lights[n].Ld * material.Kd * NdotL;
		vec3 specular	= lights[n].Ls * material.Ks * pow( RdotV, material.rShininess );
	
		return vec4((ambient * diffuse) + specular, 1.0);
	}
	else
	{
		return vec4(ambient, 1.0);
	}

}

vec4 Phong( int n, vec3 vLightDir )
{
	// we translate the terrain to -275.0 in the y direction when the scenegraph is created.
	// so if the "sun sets", we provide no light to the object
	if( lights[n].Position.y < -275.0f )
		return vec4(0, 0, 0, 1);

	vec3 vTexNormal		= normalize( ( texture2D(texBump, TexCoord).xyz * 2.0 ) - 1.0 );
	float NdotL			= max( 0.0, dot( vTexNormal, vLightDir ));

	vec3 r				= normalize( ( ( 2.0 * vTexNormal ) * NdotL ) - vLightDir );
	vec3 v				= normalize( vViewDirection );
	float RdotV			= max( 0.0, dot( r, v ) );	

	vec4	ambient		= vec4(lights[n].La * material.Ka, 1.0);
	vec4	diffuse		= vec4(lights[n].Ld * material.Kd * NdotL, 1.0);
	vec4	specular	= vec4(lights[n].Ls * material.Ks * pow( RdotV, material.rShininess ), 1.0);

	return ambient + diffuse + specular;
}

subroutine (FragmentLightIntensity)
vec4 Spotlights()
{	
	vec4 vLightIntensity;

	vLightIntensity = SpotlightPhong( 0, vec3(mSpotDirections[0]) );
	vLightIntensity += SpotlightPhong( 1, vec3(mSpotDirections[1]) );
	vLightIntensity += SpotlightPhong( 2, vec3(mSpotDirections[2]) );
	vLightIntensity += SpotlightPhong( 3, vec3(mSpotDirections[3]) );		

	return vLightIntensity;
}

subroutine( FragmentLightIntensity )
vec4 Sunlight()
{		
	return Phong( 4, vSunDirection );
}

void main()
{
	FragColour	= texture2D( tex, TexCoord ) * LightIntensity();
}