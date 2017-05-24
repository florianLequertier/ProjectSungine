#version 410 core

// Inputs : 
in block
{
    vec2 Texcoord;
} In; 

// Outputs : 
layout(location = 0, index = 0) out vec4  Color;

//Uniforms : 
uniform sampler2D Texture;

void main(void)
{
    vec4 color = texture(Texture, In.Texcoord).rgba;
    Color = color;
}
