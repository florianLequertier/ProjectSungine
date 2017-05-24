#version 410 core

#define POSITION 0

layout(location = POSITION) in vec3 Position;

uniform mat4 MVP;

void main()
{   
    gl_Position = MVP * vec4(Position.xyz, 1.0);
}
