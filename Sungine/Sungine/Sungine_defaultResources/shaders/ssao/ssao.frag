#version 410 core

// Inputs : 
in block
{
    vec2 Texcoord;
} In; 

// Outputs : 
layout(location = 0) out float  outOcclusion;

//Uniforms : 
uniform sampler2D Normals;
uniform sampler2D Depth;
uniform sampler2D NoiseTexture;
uniform vec3 Kernel[64];
uniform mat4 Projection;
uniform mat4 ScreenToView;

const float radius = 0.5;
const float bias = 0.025;

void main(void)
{
    vec2 noiseSize = textureSize(Normals, 0).xy / 4.0;
    vec3 normal = texture(Normals, In.Texcoord).rgb * 2.0 - 1.0;
    vec3 randomVector = normalize(texture(NoiseTexture, In.Texcoord * noiseSize).rgb);

    float depth = texture(Depth, In.Texcoord).r;
    // Convert texture coordinates into screen space coordinates
    vec2 xy = In.Texcoord * 2.0 -1.0;
    // Convert depth to -1,1 range and multiply the point by ScreenToWorld matrix
    vec4 wP = vec4(xy, depth * 2.0 -1.0, 1.0) * ScreenToView;
    // Divide by w
    vec3 position = vec3(wP.xyz / wP.w);

    vec3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < 64; ++i)
    {
        vec3 currentSample = TBN * Kernel[i];
        currentSample = position + currentSample * radius;

        vec4 offsetScreenSpace = vec4(currentSample, 1.0);
        offsetScreenSpace = Projection * offsetScreenSpace;
        offsetScreenSpace.xyz /= offsetScreenSpace.w;
        vec3 offset = offsetScreenSpace.xyz * 0.5 + 0.5;

        float sampleDepthInScreenSpace = texture(Depth, offset.xy).r;
        vec4 sampleDepthInViewSpace = vec4(offsetScreenSpace.xy, sampleDepthInScreenSpace * 2.0 -1.0, 1.0) * ScreenToView;
        float sampleDepth = sampleDepthInViewSpace.z / sampleDepthInViewSpace.w;

        float checkRange = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= currentSample.z + bias ? 1.0 : 0.0) * checkRange;
    }

    occlusion /= 64.f;
    occlusion *= 2.0f;
    occlusion *= occlusion;
    occlusion *= 4.0f;

    outOcclusion = occlusion;
}
