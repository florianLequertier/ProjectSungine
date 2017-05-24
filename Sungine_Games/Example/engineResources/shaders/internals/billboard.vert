#version 410 core


precision highp float;
precision highp int;

uniform vec3 Translation;
uniform vec2 Scale;
uniform vec3 CameraRight;
uniform vec3 CameraUp;

uniform vec4 ClipPlane;

uniform mat4 MVP;
//uniform mat4 NormalMatrix;
//uniform vec2 TextureRepetition;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;
layout(location = 3) in vec3 Tangent;


out block
{
        vec2 TexCoord;
        vec3 Position;
        vec4 ClipSpaceCoord;
        mat3 TBN;
} Out;

void main()
{
        vec4 posWorldSpace = vec4(Translation + CameraRight * Position.x * Scale.x + CameraUp * Position.z * Scale.y, 1.0);

        vec3 tangent = normalize(CameraRight);
        //calculate TBN matrix :
        vec3 T = normalize( vec3(vec4(tangent, 0.0)) );
        vec3 N = normalize( vec3(vec4(Normal, 0.0)) );
        vec3 B = -cross(T, N);
        Out.TBN = mat3(B, T, N);

        Out.TexCoord = TexCoord;// * TextureRepetition;
        Out.Position = posWorldSpace.xyz; // passer en view space ?

        gl_ClipDistance[0] = dot(posWorldSpace, ClipPlane);

        Out.ClipSpaceCoord = MVP * posWorldSpace;
        gl_Position = Out.ClipSpaceCoord;
}
