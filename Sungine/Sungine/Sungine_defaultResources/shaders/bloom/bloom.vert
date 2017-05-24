#version 410 core

#define POSITION 0

layout(location = POSITION) in vec2 Position;
uniform vec2 Resize;

out block
{
    vec2 Texcoord;
} Out;

void main()
{
    vec2 pos = Position * 0.5 + 0.5;
    Out.Texcoord = pos * Resize;
    gl_Position = vec4(Position.xy, 0.0, 1.0);
}
