
#version 400

// structs
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
in vec4 VertexColour;

// inputs (uniforms)
uniform vec3			vViewPosition;	// camera position (view space)
uniform LightInfo		lights[5];		// light[0-3] = spotlights, light[4] = sun
uniform MaterialInfo	material;

uniform	sampler2D		tex;
uniform mat4			mModelView;
uniform mat3			mNormal;
uniform mat4			mMVP;

// outputs 
out vec3 vVertexNormal;			// Vertex normal				(in view space)
out vec3 vViewDirection;		// direction to camera			(in view space)
out vec3 vSunDirection;			// direction to light source	(in view space)
out mat4 mSpotDirections;		// direction to each spotlight  (in view space)
out vec4 VertColour;




void main()
{
	// transform vertex position and normal into view space for Phong lighting calculation
	
	// Get vertex normal, View direction and Light direction in view space
	vec3 vVertexPos = vec3(mModelView * vec4( VertexPosition, 1.0 ));
	vec3 vLightPos;
	vec3 vLightDir;
	
	vVertexNormal	= normalize( mNormal * VertexNormal );
	vViewDirection	= normalize( vViewPosition - vVertexPos );
	vSunDirection   = normalize( vec3(lights[4].Position) - vVertexPos );

	// compute the light direction for each spotlight, store in the mSpotDirections matrix ;)
	for(int n = 0; n < 4; n ++ )
	{
		vLightPos = vec3( lights[n].Position );
		vLightDir = normalize( vLightPos - vVertexPos );

		mSpotDirections[n] = vec4(vLightDir, 1.0);
	}

	// pass vertex colour into the fragment shader
	VertColour = VertexColour;

	// all vertex shaders must set the vertex position
	gl_Position = mMVP * vec4(VertexPosition, 1.0);
}
