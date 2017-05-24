#version 410 core

#define POSITION	 0
#define NORMAL		 1
#define TEXCOORD	 2
#define TANGENT		 3
#define BONE_IDS	 4
#define BONE_WEIGHTS 5 

#define FRAG_COLOR	0

precision highp float;
precision highp int;

const unsigned int MAX_BONE_COUNT = 100;

//uniform mat4 MVP;
//uniform mat4 NormalMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform vec2 TextureRepetition;
uniform mat4 BonesTransform[MAX_BONE_COUNT];
uniform bool UseSkeleton = false;

layout(location = POSITION) in vec3 Position;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;
layout(location = TANGENT) in vec3 Tangent;
layout(location = BONE_IDS) in ivec4 BoneIds;
layout(location = BONE_WEIGHTS) in vec4 BoneWeights;


out block
{
	vec2 TexCoord; 
	vec3 Position;
	mat3 TBN;
} Out;

void main()
{	
	mat4 boneTransform = mat4(1);
	if(UseSkeleton){
	boneTransform = BonesTransform[BoneIds[0]] * BoneWeights[0];
		boneTransform += BonesTransform[BoneIds[1]] * BoneWeights[1];
		boneTransform += BonesTransform[BoneIds[2]] * BoneWeights[2];
		boneTransform += BonesTransform[BoneIds[3]] * BoneWeights[3];
	}

	vec3 pos = Position;

        mat4 normalMatrix = transpose(inverse(ViewMatrix * ModelMatrix));

        Out.TexCoord = TexCoord * TextureRepetition;
        Out.Position = vec3(ViewMatrix * boneTransform * vec4(pos, 1));

	//calculate TBN matrix : 
        vec3 T = normalize( vec3(normalMatrix * boneTransform * vec4(Tangent, 0.0)) );
        vec3 N = normalize( vec3(normalMatrix * boneTransform * vec4(Normal, 0.0)) );
	vec3 B = -cross(T, N);
	Out.TBN = mat3(B, T, N);

        gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * boneTransform * vec4(Position,1);
	
}
