
#version 400

// -- attributes
in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;

// -- outputs to fragment shader stage
out vec2 TexCoord;


// -- data-structs
struct LightInfo
{
	vec4 Position;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};

struct MaterialInfo
{
	vec3	Ka;
	vec3	Kd;
	vec3	Ks;
	float	rShininess;
};


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

	return rAmplitude[n] * sin(rTheta * rFrequency + rSimTime * rPhase );
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

vec3 WaveNormal( float x, float y )
{
	float dy = 0.0;
	float dx = 0.0;

	for( int n = 0; n < nNumWaves; n ++ )
	{
		dy += dWavedy(n, x, y);
		dx += dWavedx(n, x, y);
	}

	vec3 vWaveNormal = normalize(vec3(-dx, -dy, 1.0));
	return vWaveNormal;
}

void main()
{	
	// NB: use the keys F2-F6 to speed up the season timeline, which affects wave behaviour.
	vec4 Position	= vec4(VertexPosition, 1.0);
	
	Position.y		+= WaveHeight(Position.x, Position.z);
	Position.x		-= WaveHeight(Position.y, Position.z);
	Position.z		+= WaveHeight(Position.x, Position.y);
	Position		= vec4(Position.xyz / Position.w, 1.0);

	// Texture co-ordinate (passed to frag shader)
	TexCoord = VertexTexCoord;

	// vertex position
	gl_Position = mMVP * Position;
}
