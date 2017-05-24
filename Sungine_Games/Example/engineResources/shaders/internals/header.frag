#version 410 core

precision highp int;

// inputs :
in block
{
        vec2 TexCoord;
        vec3 Position;
        vec4 ClipSpaceCoord;
        mat3 TBN;
} In;

// Global fragment functions

void getProjectedCoord(out vec2 projectedCoords)
{
    projectedCoords = ((In.ClipSpaceCoord.xy / In.ClipSpaceCoord.w) + 1.f) * 0.5f;
}
