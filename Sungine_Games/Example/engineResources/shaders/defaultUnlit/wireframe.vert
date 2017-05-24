#version 410 core

#define POSITION	0
#define FRAG_COLOR	0

precision highp float;
precision highp int;

uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

layout(location = POSITION) in vec3 Position;


void main()
{	
        gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position,1);
}
