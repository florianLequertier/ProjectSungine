#version 410 core

#define POSITION	0
#define NORMAL		1
#define TEXCOORD	2
#define TANGENT		3
#define FRAG_COLOR	0

precision highp float;
precision highp int;


layout(location = POSITION) in vec3 Position;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;
layout(location = TANGENT) in vec3 Tangent;

uniform mat4 MVP;

void main()
{	
	gl_Position = MVP * vec4(Position,1);
}
