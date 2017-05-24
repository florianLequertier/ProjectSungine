#version 410 core

#define POSITION 0
#define COLOR 1
#define INTENSITY 2

layout(location = POSITION) in vec3 Position;
layout(location = COLOR) in vec3 Color;
layout(location = INTENSITY) in float Intensity;

out geomDatas
{
    vec4 LightPosClipSpace;
    vec3 FlareColor;
    float FlareIntensity;
} Out;

uniform mat4 VP;

void main()
{   
    Out.LightPosClipSpace = VP * vec4(Position.xyz, 1.0);
    Out.FlareColor = Color;
    Out.FlareIntensity = Intensity;

    gl_Position = vec4(Position.xyz, 1.0);
}
