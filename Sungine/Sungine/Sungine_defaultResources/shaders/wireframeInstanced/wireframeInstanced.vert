#version 410 core

#define POSITION	0
#define TRANSLATION	1
#define SCALE	2

precision highp float;
precision highp int;

layout(location = POSITION) in vec3 Position;
layout(location = TRANSLATION) in vec3 Translation;
layout(location = SCALE) in vec3 Scale;

uniform mat4 VP;

void main()
{	
	gl_Position = VP * vec4( Scale * Position + Translation, 1);
}
