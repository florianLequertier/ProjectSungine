#version 410 core

#define POSITION 0

precision highp float;
precision highp int;

layout(location = POSITION) in vec2 Position;

uniform vec2 TextureRepetition;

out block
{
    vec2 Texcoord;
	vec2 RepeatedTexcoord;
} Out;

void main()
{   
    Out.Texcoord = Position * 0.5 + 0.5;
	Out.RepeatedTexcoord = Out.Texcoord * TextureRepetition;

    gl_Position = vec4(Position.xy, 0.0, 1.0);
}