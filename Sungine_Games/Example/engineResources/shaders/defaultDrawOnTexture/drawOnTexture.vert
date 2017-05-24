#version 410 core

#define POSITION 0

precision highp float;
precision highp int;

layout(location = POSITION) in vec2 Position;

out block
{
    vec2 Texcoord;
	vec2 Position;
} Out;

void main()
{   
	Out.Position = Position;
    Out.Texcoord = Position * 0.5 + 0.5;
    gl_Position = vec4(Position.xy, 0.0, 1.0);
}