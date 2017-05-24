#version 410 core


precision highp float;
precision highp int;

#define POSITIONS 0
#define VELOCITIES 1
#define FORCES 2
#define ELAPSED_TIMES 3
#define LIFE_TIMES 4
#define COLORS 5
#define SIZES 6

layout(location = POSITIONS) in vec3 Position;
//layout(location = SIZES) in vec3 Size;
//layout(location = COLORS) in vec3 Color;

void main()
{	
	gl_Position = vec4(0,0,0,1);//vec4(Position, 1.0);
}
