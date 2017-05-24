
#define BONE_IDS	 4
#define BONE_WEIGHTS     5

const unsigned int MAX_BONE_COUNT = 100;

uniform mat4 BonesTransform[MAX_BONE_COUNT];
uniform bool UseSkeleton = false;

layout(location = BONE_IDS) in ivec4 BoneIds;
layout(location = BONE_WEIGHTS) in vec4 BoneWeights;


void computeBoneTransform(out mat4 outBoneTransform)
{
    if(UseSkeleton)
    {
        outBoneTransform = BonesTransform[BoneIds[0]] * BoneWeights[0];
        outBoneTransform += BonesTransform[BoneIds[1]] * BoneWeights[1];
        outBoneTransform += BonesTransform[BoneIds[2]] * BoneWeights[2];
        outBoneTransform += BonesTransform[BoneIds[3]] * BoneWeights[3];
    }
    else
    {
        outBoneTransform = mat4(1);
    }
}
