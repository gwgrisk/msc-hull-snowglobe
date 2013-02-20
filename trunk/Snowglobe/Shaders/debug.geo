
#version 330
precision highp float;

layout (triangles) in;
layout (line_strip) out;
layout (max_vertices = 8) out;

in Vertex
{
	vec3 Normal;
} vertex[];

uniform mat4	mMVP;
uniform float	rNormalLength;

// generate a vertex normal
for( int i = 0; i < gl_in.length(); i ++ )
{	
	gl_Position = mMVP * gl_in[i].gl_Position;
	EmitVertex();
		
	gl_Position = mMVP * vec4( gl_in[i].gl_Position.xyz + vertex[i].Normal * rNormalLength, 1.0 );
	EmitVertex();

	EndPrimitive();
}

// produce a face normal
vec4 centroid = (	gl_in[0].gl_Position + 
					gl_in[1].gl_Position + 
					gl_in[2].gl_Position ) / 3.0;

vec3 faceNormal = normalize( cross( gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, 
									gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz ) );

gl_Position = centroid * mMVP;		// is this correct?
EmitVertex();

gl_Position = ( centroid + vec4(faceNormal * rNormalLength, 0.0) ) * mMVP;
EmitVertex();

EndPrimitive();
