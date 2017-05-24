#version 410 core

#define POSITION	0
#define NORMAL		1
#define TEXCOORD	2
#define TANGENT		3
#define FRAG_COLOR	0

precision highp float;
precision highp int;

//uniform mat4 MVP;
//uniform mat4 NormalMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform vec2 TextureRepetition;

layout(location = POSITION) in vec3 Position;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;
layout(location = TANGENT) in vec3 Tangent;


out block
{
	vec2 TexCoord; 
	vec2 RepeatedTexCoord;
	vec3 Position;
	//vec3 Normal;
	mat3 TBN;
} Out;

void main()
{	
    vec3 pos = Position;

    Out.TexCoord = TexCoord;
    Out.RepeatedTexCoord = TexCoord * TextureRepetition;
    Out.Position = pos;
    //Out.Normal =  normalize( vec3(NormalMatrix * vec4(Normal, 0)) );

    mat4 normalMatrix = transpose(inverse(ViewMatrix * ModelMatrix));

    //calculate TBN matrix :
    vec3 T = normalize( vec3(normalMatrix * vec4(Tangent, 0.0)));
    vec3 N = normalize( vec3(normalMatrix * vec4(Normal, 0.0)) );
    vec3 B = -cross(T, N);
    Out.TBN = mat3(B, T, N);

    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position,1);
	
}
