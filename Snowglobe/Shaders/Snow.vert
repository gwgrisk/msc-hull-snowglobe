
#version 410

in vec3			InitialPosition;
in vec3			InitialVelocity;
in float		ParticleStartTime;

uniform float	TimeElapsed;
uniform vec3	Gravity;
uniform float	ParticleLifetime;
uniform mat4	mMVP;

out float		Transparency;


void main()
{
	vec3 Pos;
	Transparency	= 0.0f;

	float t = TimeElapsed - ParticleStartTime;

	if( TimeElapsed > ParticleStartTime )
	{		
		if( t < ParticleLifetime )
		{
			Pos				= InitialPosition + InitialVelocity * t + Gravity * t * t;
			Transparency	= 1.0f - ( t / ParticleLifetime );
		}
	}
	else
	{
		Pos = InitialPosition;
	}


	gl_Position = mMVP * vec4(Pos, 1.0);
}