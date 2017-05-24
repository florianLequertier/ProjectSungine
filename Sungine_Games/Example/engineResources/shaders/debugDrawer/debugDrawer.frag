#version 410 core

precision highp int;

layout(location = 0 ) out vec4 outColor;

in block
{
	vec3 Color;
} In;

void main()
{
	outColor = vec4( In.Color, 1.0 );
}
