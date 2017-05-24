#version 410 core

// Inputs : 
in block
{
    vec2 Texcoord;
} In; 

// Outputs : 
layout(location = 0, index = 0) out vec4  outColor;

//Uniforms : 
uniform sampler2D Texture01;
uniform sampler2D Texture02;

void main(void)
{
    outColor = vec4((texture(Texture01, In.Texcoord).rgb + texture(Texture02, In.Texcoord).rgb), 1.0);
}
