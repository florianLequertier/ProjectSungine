#version 410 core

precision highp int;

layout(location = 0 ) out vec4 outColor;

uniform vec3 Color;

void main()
{
	outColor = vec4( Color, 1.0 );
}
