
#version 400

// structs
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

// attributes
in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;

// inputs (uniforms)
uniform mat4			mModelView;
uniform mat3			mNormal;
uniform mat4			mMVP;
uniform	sampler2D		tex;
uniform sampler2D		texBump;

uniform vec3			vViewPosition;	// camera position (view space)
uniform LightInfo		lights[5];		// light[0-3] = spotlights, light[4] = sun
uniform MaterialInfo	material;


// outputs 
out vec3 vVertexNormal;			// Vertex normal				(in view space)
out vec3 vViewDirection;		// direction to camera			(in view space)
out vec3 vSunDirection;			// direction to light source	(in view space)
out mat4 mSpotDirections;		// direction to each spotlight  (in view space)
out vec2 TexCoord;				// texture coordinate			( untransformed ;) )
out vec3 vVertexPos;

// locals
vec3 vTangent		= vec3(1.0, 0.0, 0.0);


void main()
{
	// transform vertex position and normal into view space for Phong lighting calculation
	
	// Get vertex normal, View direction and Light direction in view space
	vVertexPos = vec3(mModelView * vec4( VertexPosition, 1.0 ));
	vec3 vLightPos;
	vec3 vLightDir;

	vVertexNormal	= normalize( mNormal * VertexNormal );
	vec3 vCamDir	= normalize( vViewPosition - vVertexPos );

	// compute the sun direction in tangent space
	vLightDir = normalize( vec3(lights[4].Position) - vVertexPos );

	// build tangent space basis vectors
	vec3 n,t,b,v;
	n = vVertexNormal;
	t = normalize( mNormal * vTangent );
	b = normalize(cross( n, t ));
		
	vViewDirection.x	= dot( t, vCamDir );
	vViewDirection.y	= dot( b, vCamDir );
	vViewDirection.z	= dot( n, vCamDir );
	
	vSunDirection.x		= dot( t, vLightDir.xyz );
	vSunDirection.y		= dot( b, vLightDir.xyz );
	vSunDirection.z		= dot( n, vLightDir.xyz );

	// compute the light direction for each spotlight, store in the mSpotDirections matrix ;)
	for(int n = 0; n < 4; n ++ )
	{
		vLightPos	= vec3( lights[n].Position );
		vLightDir	= normalize( vLightPos - vVertexPos );

		// build tangent space basis vectors
		t	= normalize( mNormal * vTangent );
		b	= normalize(cross( vVertexNormal, t ));
		v.x = dot( vLightDir, t );
		v.y = dot( vLightDir, b );
		v.z = dot( vLightDir, vVertexNormal );

		mSpotDirections[n] = vec4(v, 0.0);
	}
		
	// pass texture coord into the fragment shader
	TexCoord = VertexTexCoord;

	// all vertex shaders must set the vertex position
	gl_Position = mMVP * vec4(VertexPosition, 1.0);
}