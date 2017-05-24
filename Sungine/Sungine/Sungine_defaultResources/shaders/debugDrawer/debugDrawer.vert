#version 410 core

#define POSITION	0
#define COLOR	1

precision highp float;
precision highp int;

layout(location = POSITION) in vec3 Position;
layout(location = COLOR) in vec3 Color;

uniform mat4 MVP;

out block
{
	vec3 Color;
} Out;

void main()
{	
	Out.Color = Color;
	gl_Position = MVP * vec4( Position, 1);
}
