#version 410 core

#define POSITIONS 0
#define VELOCITIES 1
#define FORCES 2
#define ELAPSED_TIMES 3
#define LIFE_TIMES 4
#define COLORS 5
#define SIZES 6

layout(std140, column_major) uniform;
layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 VP;
uniform vec3 CameraUp;
uniform vec3 CameraRight;

out block
{
    vec2 TexCoord;
} Out;

void main()
{
	vec3 Pos = gl_in[0].gl_Position.xyz;
	/*
	Pos -= (CameraRight * 0.5);
	Pos += (CameraUp * 0.5);
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(0.0, 0.0);
    EmitVertex();

    Pos -= CameraUp;
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(0.0, 1.0);
    EmitVertex();

    Pos += CameraUp;
    Pos += CameraRight;
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(1.0, 0.0);
    EmitVertex();

    Pos -= CameraUp;
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(1.0, 1.0);
    EmitVertex();*/

	Pos += vec3(-0.5,0.5,0);
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(0.0, 0.0);
    EmitVertex();

    Pos += vec3(0, -1, 0);
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(0.0, 1.0);
    EmitVertex();

    Pos += vec3(1, 0, 0);
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(1.0, 0.0);
    EmitVertex();

    Pos += vec3(0, 1, 0);
    gl_Position = VP * vec4(Pos, 1.0);
    Out.TexCoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}
