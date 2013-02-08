
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

// -- attributes
in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;

// outputs 
out vec3 vVertexNormal;			// Vertex normal				(in view space)
out vec3 vViewDirection;		// direction to camera			(in view space)
out vec3 vSunDirection;			// direction to light source	(in view space)
out mat4 mSpotDirections;		// direction to each spotlight  (in view space)
out vec2 TexCoord;				// texture coordinate			( untransformed ;) )
out vec3 vVertexPos;


// -- uniforms
uniform float			rSimTime;
uniform int				nNumLights;

uniform float			rWaterHeight;		// initial offset of the lake mesh in the scene
uniform int				nNumWaves;

uniform float			rAmplitude[16];		// wave data for up to eight waves
uniform float			rWavelength[16];
uniform float			rVelocity[16];
uniform vec2			vDirection[16];

uniform vec3			vViewPosition;		// camera position transformed into view space

uniform mat4			mModelView;
uniform mat3			mNormal;
uniform mat4			mMVP;

uniform sampler2D		tex;
uniform sampler2D		texAlpha;
uniform MaterialInfo	material;
uniform LightInfo		lights[5];		// light[0-3] = spotlights, light[4] = sun

const float g_2Pi = 6.28318f;

// -- Functions

float Wave( int n, float x, float y )
{
	float rFrequency	= g_2Pi / rWavelength[n];
	
	float rPhase		= rVelocity[n] * rFrequency;
	float rTheta		= dot( vDirection[n], vec2(x, y) );

	return rAmplitude[n] * cos(rTheta * rFrequency + rSimTime * rPhase );
}

float WaveHeight( float x, float y )
{
	float rHeight = 0.0;

	for( int n = 0; n < nNumWaves; n ++ )
		rHeight += Wave(n, x, y);
	
	return rHeight;
}

float dWavedy( int n, float x, float y )
{
	float rFrequency	= g_2Pi / rWavelength[n];
	float rPhase		= rVelocity[n] * rFrequency;
	float rTheta		= dot( vDirection[n], vec2(x, y) );
	float A				= rAmplitude[n] * vDirection[n].y * rFrequency;
	return A * cos( rTheta * rFrequency + rSimTime * rPhase);

}
float dWavedx( int n, float x, float y )
{
	float rFrequency	= g_2Pi / rWavelength[n];
	float rPhase		= rVelocity[n] * rFrequency;
	float rTheta		= dot( vDirection[n], vec2(x, y) );
	float A				= rAmplitude[n] * vDirection[n].x * rFrequency;
	return A * cos( rTheta * rFrequency + rSimTime * rPhase);
}

vec3 WaveNormal( float x, float z )
{
	float dy = 0.0;
	float dx = 0.0;

	for( int n = 0; n < nNumWaves; n ++ )
	{
		dy += dWavedy(n, x, z);
		dx += dWavedx(n, x, z);
	}

	vec3 vWaveNormal = normalize(vec3(-dx, 1.0, -dy));
	return vWaveNormal;
}

void main()
{	
	// NB: use the keys F2-F6 to speed up the season timeline, which affects wave behaviour.
	vec4 Position	= vec4(VertexPosition, 1.0);
	Position.y		+= WaveHeight(Position.x, Position.z);
	Position.x		-= WaveHeight(Position.y, Position.z);
	Position.z		+= WaveHeight(Position.x, Position.y);	

	// Get the vertex position and normal into view space for phong lighting calculation
	vVertexPos		= vec3( mModelView * Position );
	vVertexNormal	= normalize( mNormal * WaveNormal(VertexNormal.x, VertexNormal.z) );
	vViewDirection	= normalize( vViewPosition - vVertexPos );
	vSunDirection   = normalize( vec3(lights[4].Position) - vVertexPos );

	// compute the light direction for each spotlight, store in the mSpotDirections matrix ;)
	for( int n = 0; n < 4; ++ n )
	{
		vec3 vLightPos		= vec3( lights[n].Position );
		vec3 vLightDir		= normalize( vLightPos - vVertexPos );
		mSpotDirections[n]	= vec4(vLightDir, 0.0);
	}

	// Texture co-ordinate (passed to frag shader)
	TexCoord = VertexTexCoord;

	// vertex shader must set the vertex position
	gl_Position = mMVP * Position;
}
