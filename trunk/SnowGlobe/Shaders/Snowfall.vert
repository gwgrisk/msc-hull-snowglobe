#version 410

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

// ShaderArgs
in vec3			VertexPosition;
in vec3			VertexVelocity;
in float		ParticleStartTime;
in vec3			VertexInitialPosition;
in vec3			VertexInitialVelocity;

// ShaderOutputs
out vec3		Position;			// Transform Feedback output
out vec3		Velocity;			// Transform Feedback output
out float		StartTime;			// Transform Feedback output
out float		Transparency;		// Frag Shader output

// Uniforms
uniform float	SimTime;
uniform vec3	Gravity;
uniform float	ParticleLifetime;
uniform mat4	mMVP;


subroutine (RenderPassType)
void update()
{
	Position	= VertexPosition;
	Velocity	= VertexVelocity;
	StartTime	= ParticleStartTime;

	float t		= SimTime - StartTime;
	vec3 at		= Gravity * t;

	if( SimTime >= StartTime )
	{		
		if( t < ParticleLifetime )
		{	
			// update			
			Position += Velocity * t;
			Velocity += at;
		}
		else
		{
			// reincarnate
			Position	= VertexInitialPosition;
			Velocity	= VertexInitialVelocity;
			StartTime	= SimTime;			
		}
	}	
}


subroutine (RenderPassType)
void render()
{
	float t			= SimTime - ParticleStartTime;
	Transparency	= 1.0 - (t / ParticleLifetime);

	gl_Position		= mMVP * vec4(VertexPosition, 1.0);
}

void main()
{
	RenderPass();
}