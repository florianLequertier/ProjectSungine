#version 410 core

#define POSITION 0

layout(location = POSITION) in vec3 Position;

uniform mat4 MVP;

out vec2 NormalizedPosition2D;

void main()
{   
    gl_Position = MVP * vec4(Position.xyz, 1.0);
    NormalizedPosition2D = gl_Position.xy;
    NormalizedPosition2D /= gl_Position.w;
    NormalizedPosition2D += 1.0f;
    NormalizedPosition2D *= 0.5f;
}
