// flat vertex shader
#version 400

subroutine vec4 FlatLightIntensity();
subroutine uniform FlatLightIntensity LightIntensity;


struct LightInfo
{
	vec4 Position;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};

struct MaterialInfo
{
	vec3  Ka;
	vec3  Kd;
	vec3  Ks;
	float rShininess;
};

// attributes
in vec3 VertexPosition;
in vec3 VertexNormal;
in vec3 VertexColour;

// uniforms
uniform vec3			vViewPosition;	// camera position (view space)
uniform LightInfo		lights[5];		// light[0-3] = spotlights, light[4] = sun
uniform MaterialInfo	material;
uniform mat4			mModelView;
uniform mat3			mNormal;
uniform mat4			mMVP;

// output to the next stage in the pipeline
flat out vec4 vLightIntensity;


// locals (saves passing them around as args!)
vec3 vViewDirection;
vec3 vSunDirection;
mat4 mSpotDirections; 
vec3 vtNormal;


// n is light index
vec3 ads( int n, vec3 vLightDir )
{
	vec3	s = vLightDir;
	vec3	v = vViewDirection;
	vec3	r = normalize(reflect( -s, vtNormal ));
	
	float	SdotN		= max( dot(s, vtNormal), 0.0 );
	vec3	ambient		= lights[n].La * material.Ka;
	vec3	diffuse		= lights[n].Ld * material.Kd * SdotN;
	vec3	specular	= vec3(0.0);

	if( SdotN > 0.0 )
	{
		specular = lights[n].Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.rShininess );
	}

	return ambient + diffuse + specular;
}

subroutine (FlatLightIntensity)
vec4 Spotlights()
{
	vec3 vIntensity;

	vIntensity =  ads( 0, vec3(mSpotDirections[0]) );
	vIntensity += ads( 1, vec3(mSpotDirections[1]) );
	vIntensity += ads( 2, vec3(mSpotDirections[2]) );
	vIntensity += ads( 3, vec3(mSpotDirections[3]) );		

	return vec4(vIntensity, 1.0);
}

subroutine (FlatLightIntensity)
vec4 Sunlight()
{
	return vec4( ads( 4, vSunDirection ), 1.0f );
}


void main()
{	
	// transform VertexPosition and VertexNormal to view space
	// (mNormal is the inverse transpose of mModelView)
	
	vec3 vVertexPos	= vec3(mModelView * vec4(VertexPosition, 1.0));
	vtNormal		= normalize( mNormal * VertexNormal );
	vViewDirection	= normalize( vViewPosition - vVertexPos );		
	vSunDirection	= normalize( vec3(lights[4].Position) - vVertexPos );
	
	vec3 vLightPos;
	vec3 vLightDir;

	for( int n = 0; n < 4; ++ n )
	{
		vLightPos = vec3( lights[n].Position );
		vLightDir = normalize( vLightPos - vVertexPos );
		
		mSpotDirections[n] = vec4( vLightDir, 0.0f );
	}

	// compute the (flat) vLightIntensity for this vertex
	vLightIntensity = LightIntensity();

	gl_Position = mMVP * vec4(VertexPosition, 1.0);
}